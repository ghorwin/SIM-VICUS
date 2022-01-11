/*
www.sourceforge.net/projects/tinyxml
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include "tinyxml.h"

#include <ctype.h>

#ifdef TIXML_USE_STL
#include <sstream>
#include <iostream>
#include <stdexcept>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#ifdef TIXML_USE_IBK_EXTENSIONS
#include <IBK_StringUtils.h>
#endif // TIXML_USE_IBK_EXTENSIONS


FILE* TiXmlFOpen( const char* filename, const char* mode );

// Note: in IBK data models merging of whitespaces should be done in user interfaces - once present in
//       project files, these must be kept.
bool TiXmlBase::condenseWhiteSpace = false;

#ifdef TIXML_USE_STL
#ifdef _WIN32

std::wstring UTF8ToWstring(const std::string& utf8str) {
	if (utf8str.empty())
		return std::wstring();

	int reslength = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, 0, 0);
	if(reslength == 0)
		throw std::logic_error("Cannot create wide string from UTF8 string.");

	std::vector<wchar_t> wide(reslength, L'\0');
	int writtenLength = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &wide[0], reslength);
	if(writtenLength == 0)
		throw std::logic_error("Cannot create wide string from UTF8 string.");

	return std::wstring(wide.begin(), wide.end());
}
#endif // _WIN32
#endif // TIXML_USE_STL

// Microsoft compiler security

// Filename is utf8-encoded except for compilers where UTF8 support and opening files with
// unicode characters is not supported. In this case filename is to be treated as an ANSI string.
FILE* TiXmlFOpen( const char* filename, const char* mode )
{
	#if defined(_MSC_VER)
		std::wstring wsfilename = UTF8ToWstring(filename);
		std::wstring wsmode = UTF8ToWstring(mode);
		FILE* fp = 0;
		fp = _wfsopen( wsfilename.c_str(), wsmode.c_str(), _SH_DENYNO );
		//errno_t err = fopen_s( &fp, filename, mode );
		return fp;
	#else
		// all UNIX/Linux variants (which support utf8) and MINGW
		return fopen( filename, mode );
	#endif
}

void TiXmlBase::EncodeString( const TIXML_STRING& str, TIXML_STRING* outString )
{
	int i=0;

	while( i<(int)str.length() )
	{
		unsigned char c = (unsigned char) str[i];

		if (    c == '&'
			 && i < ( (int)str.length() - 2 )
			 && str[i+1] == '#'
			 && str[i+2] == 'x' )
		{
			// Hexadecimal character reference.
			// Pass through unchanged.
			// &#xA9;	-- copyright symbol, for example.
			//
			// The -1 is a bug fix from Rob Laveaux. It keeps
			// an overflow from happening if there is no ';'.
			// There are actually 2 ways to exit this loop -
			// while fails (error case) and break (semicolon found).
			// However, there is no mechanism (currently) for
			// this function to return an error.
			while ( i<(int)str.length()-1 )
			{
				outString->append( str.c_str() + i, 1 );
				++i;
				if ( str[i] == ';' )
					break;
			}
		}
		else if ( c == '&' )
		{
			outString->append( entity[0].str, entity[0].strLength );
			++i;
		}
		else if ( c == '<' )
		{
			outString->append( entity[1].str, entity[1].strLength );
			++i;
		}
		else if ( c == '>' )
		{
			outString->append( entity[2].str, entity[2].strLength );
			++i;
		}
		else if ( c == '\"' )
		{
			outString->append( entity[3].str, entity[3].strLength );
			++i;
		}
		else if ( c == '\'' )
		{
			outString->append( entity[4].str, entity[4].strLength );
			++i;
		}
		else if ( c < 32 )
		{
			// Easy pass at non-alpha/numeric/symbol
			// Below 32 is symbolic.
			char buf[ 32 ];

			#if defined(TIXML_SNPRINTF)
				TIXML_SNPRINTF( buf, sizeof(buf), "&#x%02X;", (unsigned) ( c & 0xff ) );
			#else
				sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
			#endif

			//*ME:	warning C4267: convert 'size_t' to 'int'
			//*ME:	Int-Cast to make compiler happy ...
			outString->append( buf, (int)strlen( buf ) );
			++i;
		}
		else
		{
			//char realc = (char) c;
			//outString->append( &realc, 1 );
			*outString += (char) c;	// somewhat more efficient function call.
			++i;
		}
	}
}


TiXmlNode::TiXmlNode( NodeType _type ) : TiXmlBase()
{
	parent = 0;
	type = _type;
	firstChild = 0;
	lastChild = 0;
	prev = 0;
	next = 0;
}


TiXmlNode::~TiXmlNode()
{
	TiXmlNode* node = firstChild;
	TiXmlNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}
}


void TiXmlNode::CopyTo( TiXmlNode* target ) const
{
	target->SetValue (value.c_str() );
	target->userData = userData;
	target->location = location;
}


void TiXmlNode::Clear()
{
	TiXmlNode* node = firstChild;
	TiXmlNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}

	firstChild = 0;
	lastChild = 0;
}


TiXmlNode* TiXmlNode::LinkEndChild( TiXmlNode* node )
{
	assert( node->parent == 0 || node->parent == this );
	assert( node->GetDocument() == 0 || node->GetDocument() == this->GetDocument() );

	if ( node->Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		delete node;
		if ( GetDocument() )
			GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	node->parent = this;

	node->prev = lastChild;
	node->next = 0;

	if ( lastChild )
		lastChild->next = node;
	else
		firstChild = node;			// it was an empty list.

	lastChild = node;
	return node;
}


TiXmlNode* TiXmlNode::InsertEndChild( const TiXmlNode& addThis )
{
	if ( addThis.Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		if ( GetDocument() )
			GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}
	TiXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;

	return LinkEndChild( node );
}


TiXmlNode* TiXmlNode::InsertBeforeChild( TiXmlNode* beforeThis, const TiXmlNode& addThis )
{
	if ( !beforeThis || beforeThis->parent != this ) {
		return 0;
	}
	if ( addThis.Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		if ( GetDocument() )
			GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	TiXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->next = beforeThis;
	node->prev = beforeThis->prev;
	if ( beforeThis->prev )
	{
		beforeThis->prev->next = node;
	}
	else
	{
		assert( firstChild == beforeThis );
		firstChild = node;
	}
	beforeThis->prev = node;
	return node;
}


TiXmlNode* TiXmlNode::InsertAfterChild( TiXmlNode* afterThis, const TiXmlNode& addThis )
{
	if ( !afterThis || afterThis->parent != this ) {
		return 0;
	}
	if ( addThis.Type() == TiXmlNode::TINYXML_DOCUMENT )
	{
		if ( GetDocument() )
			GetDocument()->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	TiXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->prev = afterThis;
	node->next = afterThis->next;
	if ( afterThis->next )
	{
		afterThis->next->prev = node;
	}
	else
	{
		assert( lastChild == afterThis );
		lastChild = node;
	}
	afterThis->next = node;
	return node;
}


TiXmlNode* TiXmlNode::ReplaceChild( TiXmlNode* replaceThis, const TiXmlNode& withThis )
{
	if ( !replaceThis )
		return 0;

	if ( replaceThis->parent != this )
		return 0;

	if ( withThis.ToDocument() ) {
		// A document can never be a child.	Thanks to Noam.
		TiXmlDocument* document = GetDocument();
		if ( document )
			document->SetError( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	TiXmlNode* node = withThis.Clone();
	if ( !node )
		return 0;

	node->next = replaceThis->next;
	node->prev = replaceThis->prev;

	if ( replaceThis->next )
		replaceThis->next->prev = node;
	else
		lastChild = node;

	if ( replaceThis->prev )
		replaceThis->prev->next = node;
	else
		firstChild = node;

	delete replaceThis;
	node->parent = this;
	return node;
}


bool TiXmlNode::RemoveChild( TiXmlNode* removeThis )
{
	if ( !removeThis ) {
		return false;
	}

	if ( removeThis->parent != this )
	{
		assert( 0 );
		return false;
	}

	if ( removeThis->next )
		removeThis->next->prev = removeThis->prev;
	else
		lastChild = removeThis->prev;

	if ( removeThis->prev )
		removeThis->prev->next = removeThis->next;
	else
		firstChild = removeThis->next;

	delete removeThis;
	return true;
}

const TiXmlNode* TiXmlNode::FirstChild( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = firstChild; node; node = node->next )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


const TiXmlNode* TiXmlNode::LastChild( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = lastChild; node; node = node->prev )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


const TiXmlNode* TiXmlNode::IterateChildren( const TiXmlNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild();
	}
	else
	{
		assert( previous->parent == this );
		return previous->NextSibling();
	}
}


const TiXmlNode* TiXmlNode::IterateChildren( const char * val, const TiXmlNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild( val );
	}
	else
	{
		assert( previous->parent == this );
		return previous->NextSibling( val );
	}
}


const TiXmlNode* TiXmlNode::NextSibling( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = next; node; node = node->next )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


const TiXmlNode* TiXmlNode::PreviousSibling( const char * _value ) const
{
	const TiXmlNode* node;
	for ( node = prev; node; node = node->prev )
	{
		if ( strcmp( node->Value(), _value ) == 0 )
			return node;
	}
	return 0;
}


void TiXmlElement::RemoveAttribute( const char * name )
{
	#ifdef TIXML_USE_STL
	TIXML_STRING str( name );
	TiXmlAttribute* node = attributeSet.Find( str );
	#else
	TiXmlAttribute* node = attributeSet.Find( name );
	#endif
	if ( node )
	{
		attributeSet.Remove( node );
		delete node;
	}
}

const TiXmlElement* TiXmlNode::FirstChildElement() const
{
	const TiXmlNode* node;

	for (	node = FirstChild();
			node;
			node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlElement* TiXmlNode::FirstChildElement( const char * _value ) const
{
	const TiXmlNode* node;

	for (	node = FirstChild( _value );
			node;
			node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlElement* TiXmlNode::NextSiblingElement() const
{
	const TiXmlNode* node;

	for (	node = NextSibling();
			node;
			node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlElement* TiXmlNode::NextSiblingElement( const char * _value ) const
{
	const TiXmlNode* node;

	for (	node = NextSibling( _value );
			node;
			node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const TiXmlDocument* TiXmlNode::GetDocument() const
{
	const TiXmlNode* node;

	for( node = this; node; node = node->parent )
	{
		if ( node->ToDocument() )
			return node->ToDocument();
	}
	return 0;
}


TiXmlElement::TiXmlElement (const char * _value)
	: TiXmlNode( TiXmlNode::TINYXML_ELEMENT )
{
	firstChild = lastChild = 0;
	value = _value;
}


#ifdef TIXML_USE_STL
TiXmlElement::TiXmlElement( const std::string& _value )
	: TiXmlNode( TiXmlNode::TINYXML_ELEMENT )
{
	firstChild = lastChild = 0;
	value = _value;
}
#endif


TiXmlElement::TiXmlElement( const TiXmlElement& copy)
	: TiXmlNode( TiXmlNode::TINYXML_ELEMENT )
{
	firstChild = lastChild = 0;
	copy.CopyTo( this );
}


TiXmlElement& TiXmlElement::operator=( const TiXmlElement& base )
{
	ClearThis();
	base.CopyTo( this );
	return *this;
}


TiXmlElement::~TiXmlElement()
{
	ClearThis();
}


void TiXmlElement::ClearThis()
{
	Clear();
	while( attributeSet.First() )
	{
		TiXmlAttribute* node = attributeSet.First();
		attributeSet.Remove( node );
		delete node;
	}
}


const char* TiXmlElement::Attribute( const char* name ) const
{
	const TiXmlAttribute* node = attributeSet.Find( name );
	if ( node )
		return node->Value();
	return 0;
}


#ifdef TIXML_USE_STL
const std::string* TiXmlElement::Attribute( const std::string& name ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( attrib )
		return &attrib->ValueStr();
	return 0;
}
#endif


const char* TiXmlElement::Attribute( const char* name, int* i ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const char* result = 0;

	if ( attrib ) {
		result = attrib->Value();
		if ( i ) {
			attrib->QueryIntValue( i );
		}
	}
	return result;
}


#ifdef TIXML_USE_STL
const std::string* TiXmlElement::Attribute( const std::string& name, int* i ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const std::string* result = 0;

	if ( attrib ) {
		result = &attrib->ValueStr();
		if ( i ) {
			attrib->QueryIntValue( i );
		}
	}
	return result;
}
#endif


const char* TiXmlElement::Attribute( const char* name, double* d ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const char* result = 0;

	if ( attrib ) {
		result = attrib->Value();
		if ( d ) {
			attrib->QueryDoubleValue( d );
		}
	}
	return result;
}


#ifdef TIXML_USE_STL
const std::string* TiXmlElement::Attribute( const std::string& name, double* d ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	const std::string* result = 0;

	if ( attrib ) {
		result = &attrib->ValueStr();
		if ( d ) {
			attrib->QueryDoubleValue( d );
		}
	}
	return result;
}
#endif


int TiXmlElement::QueryIntAttribute( const char* name, int* ival ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryIntValue( ival );
}


int TiXmlElement::QueryUnsignedAttribute( const char* name, unsigned* value ) const
{
	const TiXmlAttribute* node = attributeSet.Find( name );
	if ( !node )
		return TIXML_NO_ATTRIBUTE;

	int ival = 0;
	int result = node->QueryIntValue( &ival );
	*value = (unsigned)ival;
	return result;
}


int TiXmlElement::QueryBoolAttribute( const char* name, bool* bval ) const
{
	const TiXmlAttribute* node = attributeSet.Find( name );
	if ( !node )
		return TIXML_NO_ATTRIBUTE;

	int result = TIXML_WRONG_TYPE;
	if (    StringEqual( node->Value(), "true", true, TIXML_ENCODING_UNKNOWN )
		 || StringEqual( node->Value(), "yes", true, TIXML_ENCODING_UNKNOWN )
		 || StringEqual( node->Value(), "1", true, TIXML_ENCODING_UNKNOWN ) )
	{
		*bval = true;
		result = TIXML_SUCCESS;
	}
	else if (    StringEqual( node->Value(), "false", true, TIXML_ENCODING_UNKNOWN )
			  || StringEqual( node->Value(), "no", true, TIXML_ENCODING_UNKNOWN )
			  || StringEqual( node->Value(), "0", true, TIXML_ENCODING_UNKNOWN ) )
	{
		*bval = false;
		result = TIXML_SUCCESS;
	}
	return result;
}



#ifdef TIXML_USE_STL
int TiXmlElement::QueryIntAttribute( const std::string& name, int* ival ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryIntValue( ival );
}
#endif


int TiXmlElement::QueryDoubleAttribute( const char* name, double* dval ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryDoubleValue( dval );
}


#ifdef TIXML_USE_STL
int TiXmlElement::QueryDoubleAttribute( const std::string& name, double* dval ) const
{
	const TiXmlAttribute* attrib = attributeSet.Find( name );
	if ( !attrib )
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryDoubleValue( dval );
}
#endif


void TiXmlElement::SetAttribute( const char * name, int val )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetIntValue( val );
	}
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute( const std::string& name, int val )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetIntValue( val );
	}
}
#endif


void TiXmlElement::SetDoubleAttribute( const char * name, double val )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetDoubleValue( val );
	}
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetDoubleAttribute( const std::string& name, double val )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( name );
	if ( attrib ) {
		attrib->SetDoubleValue( val );
	}
}
#endif


void TiXmlElement::SetAttribute( const char * cname, const char * cvalue )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( cname );
	if ( attrib ) {
		attrib->SetValue( cvalue );
	}
}


#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute( const std::string& _name, const std::string& _value )
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate( _name );
	if ( attrib ) {
		attrib->SetValue( _value );
	}
}
#endif


void TiXmlElement::Print( FILE* cfile, int depth ) const
{
	int i;
	assert( cfile );
	for ( i=0; i<depth; i++ ) {
		fprintf( cfile, "\t" );
	}

	fprintf( cfile, "<%s", value.c_str() );

	const TiXmlAttribute* attrib;
	for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
	{
		fprintf( cfile, " " );
		attrib->Print( cfile, depth );
	}

	// There are 3 different formatting approaches:
	// 1) An element without children is printed as a <foo /> node
	// 2) An element with only a text child is printed as <foo> text </foo>
	// 3) An element with children is printed on multiple lines.
	TiXmlNode* node;
	if ( !firstChild )
	{
		fprintf( cfile, " />" );
	}
	else if ( firstChild == lastChild && firstChild->ToText() )
	{
		fprintf( cfile, ">" );
		firstChild->Print( cfile, depth + 1 );
		fprintf( cfile, "</%s>", value.c_str() );
	}
	else
	{
		fprintf( cfile, ">" );

		for ( node = firstChild; node; node=node->NextSibling() )
		{
			if ( !node->ToText() )
			{
				fprintf( cfile, "\n" );
			}
			node->Print( cfile, depth+1 );
		}
		fprintf( cfile, "\n" );
		for( i=0; i<depth; ++i ) {
			fprintf( cfile, "\t" );
		}
		fprintf( cfile, "</%s>", value.c_str() );
	}
}


void TiXmlElement::CopyTo( TiXmlElement* target ) const
{
	// superclass:
	TiXmlNode::CopyTo( target );

	// Element class:
	// Clone the attributes, then clone the children.
	const TiXmlAttribute* attribute = 0;
	for(	attribute = attributeSet.First();
	attribute;
	attribute = attribute->Next() )
	{
		target->SetAttribute( attribute->Name(), attribute->Value() );
	}

	TiXmlNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		target->LinkEndChild( node->Clone() );
	}
}

bool TiXmlElement::Accept( TiXmlVisitor* visitor ) const
{
	if ( visitor->VisitEnter( *this, attributeSet.First() ) )
	{
		for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
		{
			if ( !node->Accept( visitor ) )
				break;
		}
	}
	return visitor->VisitExit( *this );
}

#ifdef TIXML_USE_IBK_EXTENSIONS

TiXmlElement * TiXmlElement::appendSingleAttributeElement( TiXmlElement * parent,
	const char * const elementName, const char * const attribName,
	const std::string & attribValue, const std::string & elementValue)
{
	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( elementName );
	parent->LinkEndChild(xmlElement);

	// set attribute if any
	if (attribName != NULL) {
		xmlElement->SetAttribute(attribName, attribValue);
	}
	// add value if any
	if (!elementValue.empty()) {
		TiXmlText * text = new TiXmlText( elementValue );
		xmlElement->LinkEndChild( text );
	}
	return xmlElement;
}


void TiXmlElement::readSingleAttributeElement( const TiXmlElement * element,
	const char * const attribName, std::string & attribValue,
	std::string & elementValue, bool attributeIsOptional)
{
	if (attribName != NULL) {
		const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, attribName);
		if (attrib == NULL) {
			if (!attributeIsOptional) {
				std::stringstream strm;
				strm << "Error in XML file, line " << element->Row() << ": ";
				strm << "Missing '" << attribName << "' attribute in element.";
				throw std::runtime_error(strm.str());
			}
		}
		else {
			attribValue = attrib->Value();
		}
	}
	const char * const str = element->GetText();
	if (str)
		elementValue = str;
	else
		elementValue.clear();
}


void TiXmlElement::readSingleAttributeElement(	const TiXmlElement * parent,
												const char * const subElementName,
												const char * const attribName,
												std::string & attribValue,
												std::string & elementValue
												)
{
	const TiXmlElement * element = parent->FirstChildElement( subElementName );
	if (element == NULL) {
		std::stringstream strm;
		strm << "Error in XML file, line " << parent->Row() << ": ";
		strm << "Missing child element '" << subElementName << "'.";
		throw std::runtime_error(strm.str());
	}
	if (attribName != NULL) {
		const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, attribName);
		// attributes are optional by default, attribValue is cleared when attribute is missing
		if (attrib != NULL)
			attribValue = attrib->Value();
		else
			attribValue.clear();
	}
	const char * const str = element->GetText();
	if (str)
		elementValue = str;
	else
		elementValue.clear();
}


void TiXmlElement::appendIBKParameterElement( TiXmlElement * parent,
	const std::string & name, const std::string & unit, double value, bool useEmbeddedForm)
{
	// first create element as child of parent
	TiXmlElement * xmlElement;
	if (useEmbeddedForm)
		xmlElement = new TiXmlElement( name );
	else
		xmlElement = new TiXmlElement( "IBK:Parameter" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if (!useEmbeddedForm)
		xmlElement->SetAttribute("name", name);
	xmlElement->SetAttribute("unit", unit);
	std::stringstream para;
	para.precision(10);
	para << value;
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::appendIBKParameterElement( TiXmlElement * parent,
	const std::string & name, const std::string & unit, unsigned int value, bool useEmbeddedForm)
{
	TiXmlElement * xmlElement;
	if (useEmbeddedForm)
		xmlElement = new TiXmlElement( name );
	else
		xmlElement = new TiXmlElement( "IBK:Parameter" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if (!useEmbeddedForm)
		xmlElement->SetAttribute("name", name);
	if (!unit.empty())
		xmlElement->SetAttribute("unit", unit);
	std::stringstream para;
	para << value;
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::appendIBKParameterElement( TiXmlElement * parent,
	const std::string & name, const std::string & unit, int value, bool useEmbeddedForm)
{
	// first create element as child of parent
	TiXmlElement * xmlElement;
	if (useEmbeddedForm)
		xmlElement = new TiXmlElement( name );
	else
		xmlElement = new TiXmlElement( "IBK:Parameter" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if (!useEmbeddedForm)
		xmlElement->SetAttribute("name", name);
	if (!unit.empty())
		xmlElement->SetAttribute("unit", unit);
	std::stringstream para;
	para << value;
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::readIBKParameterElement( const TiXmlElement * element, std::string & name,
	std::string & unit, double & value)
{
	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "name");
	if (attrib == NULL){
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Missing 'name' attribute in IBK:Parameter element.";
		throw std::runtime_error(strm.str());
	}
	name = attrib->Value();
	attrib = TiXmlAttribute::attributeByName(element, "unit");
	if (attrib == NULL) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Missing 'unit' attribute in IBK:Parameter element.";
		throw std::runtime_error(strm.str());
	}
	unit = attrib->Value();

	const char * const str = element->GetText();
	std::string valstr;
	if (str)		valstr = str;
	else			valstr.clear();
	try {
		// NOTE: reading the parameter with stringstream is not very safe - values like "1,433" will be
		//       read as 1 without raising an error. Hence, we use the IBK::string2val<> function.
		value = IBK::string2val<double>(valstr);
	} catch (IBK::Exception & ex) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Cannot read value in IBK:Parameter element.";
		strm << ex.what();
		throw std::runtime_error(strm.str());
	}
}


void TiXmlElement::appendIBKUnitVectorElement( TiXmlElement * parent,
										  const std::string & name,
										  const std::string & unit,
										  const std::vector<double> & data,
										  bool useEmbeddedForm)
{
	// first create element as child of parent
	TiXmlElement * xmlElement;
	if (useEmbeddedForm)
		xmlElement = new TiXmlElement( name );
	else
		xmlElement = new TiXmlElement( "IBK:UnitVector" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if (!useEmbeddedForm)
		xmlElement->SetAttribute("name", name);
	xmlElement->SetAttribute("unit", unit);
	std::stringstream para;
	for (unsigned int i=0; i<data.size(); ++i)
		para << data[i] << " ";
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::readIBKUnitVectorElement( const TiXmlElement * element,
									  std::string & name,
									  std::string & unit,
									  std::vector<double> & data,
									  bool useEmbeddedForm)
{
	// read name attribute, unless using embedded for
	const TiXmlAttribute* attrib;
	if (!useEmbeddedForm) {
		attrib = TiXmlAttribute::attributeByName(element, "name");
		if (attrib == NULL) {
			std::stringstream strm;
			strm << "Error in XML file, line " << element->Row() << ": ";
			strm << "Missing 'name' attribute in IBK:UnitVector element.";
			throw std::runtime_error(strm.str());
		}
		name = attrib->Value();
	}
	attrib = TiXmlAttribute::attributeByName(element, "unit");
	if (attrib == NULL) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Missing 'unit' attribute in IBK:UnitVector element.";
		throw std::runtime_error(strm.str());
	}
	unit = attrib->Value();

	const char * const str = element->GetText();
	std::string valstr;
	if (str) {
		valstr = str;
		try {
			IBK::string2valueVector(valstr, data);
		} catch (IBK::Exception & ex) {
			std::stringstream strm;
			strm << "Error in XML file, line " << element->Row() << ": ";
			strm << ex.what();
			throw std::runtime_error(strm.str());
		}
	}
	else
		data.clear();
}


void TiXmlElement::appendIBKDateElement(TiXmlElement * parent,
										const std::string & name,
										const std::string & langId,
										const std::string & dateString,
										bool useEmbeddedForm)
{
	// first create element as child of parent
	TiXmlElement * xmlElement;
	if (useEmbeddedForm)
		xmlElement = new TiXmlElement( name );
	else
		xmlElement = new TiXmlElement( "IBK:Date" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if (!useEmbeddedForm)
		xmlElement->SetAttribute("name", name);
	if (!langId.empty())
		xmlElement->SetAttribute("languageFormat", langId);
	TiXmlText * text = new TiXmlText( dateString );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::readIBKDateElement(	const TiXmlElement * parent,
										std::string & name,
										std::string & langId,
										std::string & dateString,
										bool useEmbeddedForm)
{
		// read name attribute, unless using embedded for
	const TiXmlAttribute* attrib;
	if (!useEmbeddedForm) {
		attrib = TiXmlAttribute::attributeByName(parent, "name");
		if (attrib == NULL) {
			std::stringstream strm;
			strm << "Error in XML file, line " << parent->Row() << ": ";
			strm << "Missing 'name' attribute in IBK:Date element.";
			throw std::runtime_error(strm.str());
		}
		name = attrib->Value();
	}
	attrib = TiXmlAttribute::attributeByName(parent, "languageFormat");
	if (attrib != NULL) {
		langId = attrib->Value();
	}

	dateString = parent->GetText();
}


void TiXmlElement::appendIBKVectorElement( TiXmlElement * parent,
										  const std::string & name,
										  const std::vector<double> & data)
{
	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( name );
	parent->LinkEndChild(xmlElement);

	std::stringstream para;
	for (unsigned int i=0; i<data.size(); ++i)
		para << data[i] << " ";
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::appendIBKVectorElement( TiXmlElement * parent,
										  const std::string & name,
										  const std::vector<unsigned int> & data)
{
	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( name );
	parent->LinkEndChild(xmlElement);

	std::stringstream para;
	for (unsigned int i=0; i<data.size(); ++i)
		para << data[i] << " ";
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::appendIBKVectorElement( TiXmlElement * parent,
										  const std::string & name,
										  const std::vector<int> & data)
{
	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( name );
	parent->LinkEndChild(xmlElement);

	std::stringstream para;
	for (unsigned int i=0; i<data.size(); ++i)
		para << data[i] << " ";
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::readIBKVectorElement( const TiXmlElement * element,
									  std::vector<double> & data)
{
	const char * const str = element->GetText();
	std::string valstr;
	if (str)		valstr = str;
	else			valstr.clear();
	std::stringstream sstrm(valstr);
	data.clear();
	std::string s;
	while (sstrm >> s) {
		std::stringstream valstr(s);
		double v;
		if (!(valstr >> v)) {
			std::stringstream strm;
			strm << "Error in XML file, line " << element->Row() << ": ";
			strm << "Invalid floating point value '" << s << "' in vector.";
			throw std::runtime_error(strm.str());
		}

		data.push_back(v);
	}
}


void TiXmlElement::readIBKVectorElement( const TiXmlElement * element,
									  std::vector<unsigned int> & data)
{
	const char * const str = element->GetText();
	std::string valstr;
	if (str)		valstr = str;
	else			valstr.clear();
	std::stringstream sstrm(valstr);
	data.clear();
	std::string s;
	while (sstrm >> s) {
		std::stringstream valstr(s);
		unsigned int v;
		if (!(valstr >> v)) {
			std::stringstream strm;
			strm << "Error in XML file, line " << element->Row() << ": ";
			strm << "Invalid unsigned integer value '" << s << "' in vector.";
			throw std::runtime_error(strm.str());
		}

		data.push_back(v);
	}
}


void TiXmlElement::readIBKVectorElement( const TiXmlElement * element,
									  std::vector<int> & data)
{
	const char * const str = element->GetText();
	std::string valstr;
	if (str)		valstr = str;
	else			valstr.clear();
	std::stringstream sstrm(valstr);
	data.clear();
	std::string s;
	while (sstrm >> s) {
		std::stringstream valstr(s);
		int v;
		if (!(valstr >> v)) {
			std::stringstream strm;
			strm << "Error in XML file, line " << element->Row() << ": ";
			strm << "Invalid unsigned integer value '" << s << "' in vector.";
			throw std::runtime_error(strm.str());
		}

		data.push_back(v);
	}
}


void TiXmlElement::appendIBKLinearSplineElement( TiXmlElement * parent,
										  const std::string & name,
										  const std::string & interpolationMethod,
										  const std::string & xunit,
										  const std::vector<double> & xdata,
										  const std::string & yunit,
										  const std::vector<double> & ydata)
{
	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( "IBK:LinearSpline" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	xmlElement->SetAttribute("name", name);
	if (!interpolationMethod.empty())
		xmlElement->SetAttribute("interpolation", interpolationMethod);
	TiXmlElement::appendIBKUnitVectorElement(xmlElement, "X", xunit, xdata, true);
	TiXmlElement::appendIBKUnitVectorElement(xmlElement, "Y", yunit, ydata, true);
}


void TiXmlElement::readIBKLinearSplineElement( const TiXmlElement * element,
									 std::string & name,
									 std::string & interpolationMethod,
									 std::string & xunit,
									 std::vector<double> & xdata,
									 std::string & yunit,
									 std::vector<double> & ydata)
{
	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "name");
	if (attrib == NULL) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Missing 'name' attribute in IBK:LinearSpline element.";
		throw std::runtime_error(strm.str());
	}
	name = attrib->Value();
	attrib = TiXmlAttribute::attributeByName(element, "interpolation");
	if (attrib != NULL) {
		interpolationMethod = attrib->Value();
	}
	else
		interpolationMethod.clear();
	for (const TiXmlElement * e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {
		std::string ename = e->Value();
		std::string name;
		if (ename == "X") {
			TiXmlElement::readIBKUnitVectorElement(e, name, xunit, xdata, true);
		}
		else if (ename == "Y") {
			TiXmlElement::readIBKUnitVectorElement(e, name, yunit, ydata, true);
		}
		else {
			std::stringstream strm;
			strm << "Error in XML file, line " << e->Row() << ": ";
			strm << "Undefined child element '"<< ename << "' in IBK:LinearSpline element, should be either X or Y.";
			throw std::runtime_error(strm.str());
		}
	}
}


void TiXmlElement::readIBKLinearSplineParameterElement(const TiXmlElement *element,
														std::string &name,
														std::string &interpolationMethod,
														std::string &xunit,
														std::vector<double> &xdata,
														std::string &yunit,
														std::vector<double> &ydata,
														std::string &path)

{
	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "name");
	if (attrib == NULL) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Missing 'name' attribute in linear spline element.";
		throw std::runtime_error(strm.str());
	}
	name = attrib->Value();
	attrib = TiXmlAttribute::attributeByName(element, "interpolation");
	if (attrib != NULL) {
		interpolationMethod = attrib->Value();
	}
	else
		interpolationMethod.clear();
	for (const TiXmlElement * e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {
		std::string ename = e->Value();
		std::string name;
		if (ename == "X") {
			TiXmlElement::readIBKUnitVectorElement(e, name, xunit, xdata, true);
		}
		else if (ename == "Y") {
			TiXmlElement::readIBKUnitVectorElement(e, name, yunit, ydata, true);
		}
		else if (ename == "TSVFile") {
			const TiXmlNode * child = e->FirstChild();
			if (child == NULL){
				std::stringstream strm;
				strm << "Error in XML file, line " << e->Row() << ": ";
				strm << "Missing 'TSVFile' element";
				throw std::runtime_error(strm.str());
			}
			else{
				path = child->Value();
			}
		}
		else {
			std::stringstream strm;
			strm << "Error in XML file, line " << e->Row() << ": ";
			strm << "Undefined child element '"<< ename << "' in linear spline element, should be either 'X', 'Y' or 'TSVFile'.";
			throw std::runtime_error(strm.str());
		}
	}
	if (xdata.empty() && ydata.empty() && path.empty()) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Expected either X and Y, or TSVFile tag within linear spline element.";
		throw std::runtime_error(strm.str());
	}
	// if we have x, we also need y
	if (xdata.size() != ydata.size() ) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "X and Y vectors in linear spline element must be both present and have the same number of values.";
		throw std::runtime_error(strm.str());
	}
	// we have an exclusive "or" on XY or TSVFile
	if (!xdata.empty() && !path.empty()) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Either X and Y, or TSVFile tag must be defined within linear spline element, bot not both.";
		throw std::runtime_error(strm.str());
	}
}


void TiXmlElement::appendIBKPoint3DElement( TiXmlElement * parent,
											const unsigned int & id,
											const std::string & hint,
											double x, double y, double z)
{
	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( "IBK:Point3D" );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if ( id != 0)
		xmlElement->SetAttribute("id", id);
	if (!hint.empty())
		xmlElement->SetAttribute("hint", hint);
	std::stringstream para;
	para << x << " " << y << " " << z;
	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );
}


void TiXmlElement::readIBKPoint3DElement( const TiXmlElement * element,
											unsigned int & id,
											std::string & hint,
											double & x, double & y, double & z)
{
	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "id");
	if (attrib != NULL) {
		std::stringstream strm2(attrib->Value());
		if (!(strm2 >> id)) {
			std::stringstream strm;
			strm << "Error in XML file, line " << element->Row() << ": ";
			strm << "Invalid integer value '"<< attrib->Value() << "' for 'id' attribute in IBK:Point3D element.";
			throw std::runtime_error(strm.str());
		}
	}
	else {
		/// \todo this doesn't fit to append rule -> application must check if id !=0  and hint is given! see specification for delphin
		id=0;
//		std::stringstream strm;
//		strm << "Warning in XML file, line " << element->Row() << ": ";
//		strm << "Missing 'id' attribute in IBK:Point3D element.";
//		throw std::runtime_error(strm.str());
	}

	const TiXmlAttribute* attrib2 = TiXmlAttribute::attributeByName(element, "hint");
	if (attrib2 != NULL) {
		hint = attrib2->Value();
	}
	else {
		hint.clear();
//		std::stringstream strm;
//		strm << "Error in XML file, line " << element->Row() << ": ";
//		strm << "Missing 'hint' attribute in IBK:Point3D element.";
//		throw std::runtime_error(strm.str());
	}

	std::string valstr = element->GetText();
	std::stringstream strm(valstr);
	if (!(strm >> x >> y >> z)) {
		std::stringstream strm;
		strm << "Error in XML file, line " << element->Row() << ": ";
		strm << "Invalid value/encoding '"<< valstr<< "' of IBK:Point3D value.";
		throw std::runtime_error(strm.str());
	}
}


void TiXmlElement::appendIBKRotationMatrixElement(	TiXmlElement * parent,
													const std::string & name,
													const int & MatrixElementID,
													const std::vector< std::vector< double > > & data)
{

	if (parent == NULL)
		return; // TODO : Error handling

	// first create element as child of parent
	TiXmlElement * xmlElement = new TiXmlElement( name );
	parent->LinkEndChild(xmlElement);

	// set attributes
	if ( MatrixElementID != 0)
		xmlElement->SetAttribute("elementID", MatrixElementID);

	std::stringstream para;
	for (unsigned int i=0; i<data.size(); ++i)
		for (unsigned int k=0; k<data[i].size();++k)
			para << data[i][k] << " ";

	TiXmlText * text = new TiXmlText( para.str() );
	xmlElement->LinkEndChild( text );

}



void TiXmlElement::readIBKRotationMatrixElement(	const TiXmlElement * element,
													int & MatrixElementID,
													std::vector< std::vector< double > > & data)
{

	if (element == NULL)
		return; // TODO : Error handling

	if ( data.size() != 3 )
		return;

	if ( data[0].size() != 3 || data[1].size() != 3 || data[2].size() != 3  )
		return;

	const TiXmlAttribute* attrib = TiXmlAttribute::attributeByName(element, "element");
	if (attrib == NULL)
		throw std::runtime_error("Missing 'element' attribute in IBK:RotationMatrix element.");
	std::stringstream strm2(attrib->Value());
	strm2 >> MatrixElementID;

	std::string valstr = element->GetText();
	std::stringstream strm(valstr);
	data.clear();
	std::vector< double > row;
	int v;
	int i = 3, k = 3;

	while (i){
		while (k){
			if (strm >> v){
				row.push_back( v );
			} // if (strm >> v)
			--k;
		} // while(k)
		data.push_back(row);
		--i;
	} // while (i)

}





int TiXmlElement::dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent)
{
	if ( !pElement ) return 0;

	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
	int i=0;
	int ival;
	double dval;
	const char* pIndent=getIndent(indent);
	printf("\n");
	while (pAttrib)
	{
		printf( "%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());

		if (pAttrib->QueryIntValue(&ival)==TIXML_SUCCESS)    printf( " int=%d", ival);
		if (pAttrib->QueryDoubleValue(&dval)==TIXML_SUCCESS) printf( " d=%1.1f", dval);
		printf( "\n" );
		i++;
		pAttrib=pAttrib->Next();
	}
	return i;
}


void TiXmlElement::dump_to_stdout( TiXmlNode* pParent, unsigned int indent )
{
	if ( !pParent ) return;

	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	printf( "%s", getIndent(indent));
	int num;

	switch ( t )
	{
	case TiXmlNode::TINYXML_DOCUMENT:
		printf( "Document" );
		break;

	case TiXmlNode::TINYXML_ELEMENT:
		printf( "Element [%s]", pParent->Value() );
		num=dump_attribs_to_stdout(pParent->ToElement(), indent+1);
		switch(num)
		{
			case 0:  printf( " (No attributes)"); break;
			case 1:  printf( "%s1 attribute", getIndentAlt(indent)); break;
			default: printf( "%s%d attributes", getIndentAlt(indent), num); break;
		}
		break;

	case TiXmlNode::TINYXML_COMMENT:
		printf( "Comment: [%s]", pParent->Value());
		break;

	case TiXmlNode::TINYXML_UNKNOWN:
		printf( "Unknown" );
		break;

	case TiXmlNode::TINYXML_TEXT:
		pText = pParent->ToText();
		printf( "Text: [%s]", pText->Value() );
		break;

	case TiXmlNode::TINYXML_DECLARATION:
		printf( "Declaration" );
		break;
	default:
		break;
	}
	printf( "\n" );
	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		dump_to_stdout( pChild, indent+1 );
	}
}

// load the named file and dump its structure to STDOUT
void TiXmlElement::dump_to_stdout(const char* pFilename)
{
	TiXmlDocument doc(pFilename);
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
	{
		printf("\n%s:\n", pFilename);
		dump_to_stdout( &doc ); // defined later in the tutorial
	}
	else
	{
		printf("Failed to load file \"%s\"\n", pFilename);
	}
}


bool TiXmlElement::killAllComments( TiXmlNode* pParent  ){

	// nothing todo -> exit
	if ( !pParent )
		return false;

	// we found a comment so we remove the structure
	if (pParent->Type() == TiXmlNode::TINYXML_COMMENT){
		return true;
	}

	// recursive call
	TiXmlNode* pChild = pParent->FirstChild();
	TiXmlNode* pTmpChild;
	while (  pChild != 0 )
	{
		pTmpChild = pChild->NextSibling();
		if (killAllComments( pChild )){
			pParent->RemoveChild(pChild);
		}
		pChild = pTmpChild;
	}

	return false;
}



const unsigned int NUM_INDENTS_PER_SPACE=2;

const char * TiXmlElement::getIndent( unsigned int numIndents )
{
	static const char * pINDENT="                                      + ";
	static const unsigned int LENGTH=strlen( pINDENT );
	unsigned int n=numIndents*NUM_INDENTS_PER_SPACE;
	if ( n > LENGTH ) n = LENGTH;

	return &pINDENT[ LENGTH-n ];
}


const char * TiXmlElement::getIndentAlt( unsigned int numIndents )
{
	static const char * pINDENT="                                        ";
	static const unsigned int LENGTH=strlen( pINDENT );
	unsigned int n=numIndents*NUM_INDENTS_PER_SPACE;
	if ( n > LENGTH ) n = LENGTH;

	return &pINDENT[ LENGTH-n ];
}

#endif // TIXML_USE_IBK_EXTENSIONS


TiXmlNode* TiXmlElement::Clone() const
{
	TiXmlElement* clone = new TiXmlElement( Value() );
	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


const char* TiXmlElement::GetText() const
{
	const TiXmlNode* child = this->FirstChild();
	if ( child ) {
		const TiXmlText* childText = child->ToText();
		if ( childText ) {
			return childText->Value();
		}
	}
	return "";
}


const char* TiXmlElement::GetTextUnsafe() const
{
	const TiXmlNode* child = this->FirstChild();
	if ( child ) {
		const TiXmlText* childText = child->ToText();
		if ( childText ) {
			return childText->Value();
		}
	}
	return 0;
}


TiXmlDocument::TiXmlDocument() : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	ClearError();
}

TiXmlDocument::TiXmlDocument( const char * documentName ) : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	value = documentName;
	ClearError();
}


#ifdef TIXML_USE_STL
TiXmlDocument::TiXmlDocument( const std::string& documentName ) : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	value = documentName;
	ClearError();
}
#endif


TiXmlDocument::TiXmlDocument( const TiXmlDocument& copy ) : TiXmlNode( TiXmlNode::TINYXML_DOCUMENT )
{
	copy.CopyTo( this );
}


TiXmlDocument& TiXmlDocument::operator=( const TiXmlDocument& copy )
{
	Clear();
	copy.CopyTo( this );
	return *this;
}


bool TiXmlDocument::LoadFile( TiXmlEncoding encoding )
{
	return LoadFile( Value(), encoding );
}


bool TiXmlDocument::SaveFile() const
{
	return SaveFile( Value() );
}

bool TiXmlDocument::LoadFile( const char* _filename, TiXmlEncoding encoding )
{
	TIXML_STRING filename( _filename );
	value = filename;

	// reading in binary mode so that tinyxml can normalize the EOL
	FILE* file = TiXmlFOpen( value.c_str (), "rb" );

	if ( file )
	{
		bool result = LoadFile( file, encoding );
		fclose( file );
		return result;
	}
	else
	{
		SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}
}

bool TiXmlDocument::LoadFile( FILE* file, TiXmlEncoding encoding )
{
	if ( !file )
	{
		SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Delete the existing data:
	Clear();
	location.Clear();

	// Get the file size, so we can pre-allocate the string. HUGE speed impact.
	long length = 0;
	fseek( file, 0, SEEK_END );
	length = ftell( file );
	fseek( file, 0, SEEK_SET );

	// Strange case, but good to handle up front.
	if ( length <= 0 )
	{
		SetError( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Subtle bug here. TinyXml did use fgets. But from the XML spec:
	// 2.11 End-of-Line Handling
	// <snip>
	// <quote>
	// ...the XML processor MUST behave as if it normalized all line breaks in external
	// parsed entities (including the document entity) on input, before parsing, by translating
	// both the two-character sequence #xD #xA and any #xD that is not followed by #xA to
	// a single #xA character.
	// </quote>
	//
	// It is not clear fgets does that, and certainly isn't clear it works cross platform.
	// Generally, you expect fgets to translate from the convention of the OS to the c/unix
	// convention, and not work generally.

	/*
	while( fgets( buf, sizeof(buf), file ) )
	{
		data += buf;
	}
	*/

	char* buf = new char[ length+1 ];
	buf[0] = 0;

	if ( fread( buf, length, 1, file ) != 1 ) {
		delete [] buf;
		SetError( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Process the buffer in place to normalize new lines. (See comment above.)
	// Copies from the 'p' to 'q' pointer, where p can advance faster if
	// a newline-carriage return is hit.
	//
	// Wikipedia:
	// Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or
	// CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
	//		* LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
	//		* CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
	//		* CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

	const char* p = buf;	// the read head
	char* q = buf;			// the write head
	const char CR = 0x0d;
	const char LF = 0x0a;

	buf[length] = 0;
	while( *p ) {
		assert( p < (buf+length) );
		assert( q <= (buf+length) );
		assert( q <= p );

		if ( *p == CR ) {
			*q++ = LF;
			p++;
			if ( *p == LF ) {		// check for CR+LF (and skip LF)
				p++;
			}
		}
		else {
			*q++ = *p++;
		}
	}
	assert( q <= (buf+length) );
	*q = 0;

	Parse( buf, 0, encoding );

	delete [] buf;
	return !Error();
}


bool TiXmlDocument::SaveFile( const char * filename ) const
{
	// The old c stuff lives on...
	FILE* fp = TiXmlFOpen( filename, "w" );
	if ( fp )
	{
		bool result = SaveFile( fp );
		fclose( fp );
		return result;
	}
	return false;
}


bool TiXmlDocument::SaveFile( FILE* fp ) const
{
	if ( useMicrosoftBOM )
	{
		const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
		const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
		const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

		fputc( TIXML_UTF_LEAD_0, fp );
		fputc( TIXML_UTF_LEAD_1, fp );
		fputc( TIXML_UTF_LEAD_2, fp );
	}
	Print( fp, 0 );
	return (ferror(fp) == 0);
}


void TiXmlDocument::CopyTo( TiXmlDocument* target ) const
{
	TiXmlNode::CopyTo( target );

	target->error = error;
	target->errorId = errorId;
	target->errorDesc = errorDesc;
	target->tabsize = tabsize;
	target->errorLocation = errorLocation;
	target->useMicrosoftBOM = useMicrosoftBOM;

	TiXmlNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		target->LinkEndChild( node->Clone() );
	}
}


TiXmlNode* TiXmlDocument::Clone() const
{
	TiXmlDocument* clone = new TiXmlDocument();
	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


void TiXmlDocument::Print( FILE* cfile, int depth ) const
{
	assert( cfile );
	for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
	{
		node->Print( cfile, depth );
		fprintf( cfile, "\n" );
	}
}


bool TiXmlDocument::Accept( TiXmlVisitor* visitor ) const
{
	if ( visitor->VisitEnter( *this ) )
	{
		for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
		{
			if ( !node->Accept( visitor ) )
				break;
		}
	}
	return visitor->VisitExit( *this );
}


const TiXmlAttribute* TiXmlAttribute::Next() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( next->value.empty() && next->name.empty() )
		return 0;
	return next;
}

/*
TiXmlAttribute* TiXmlAttribute::Next()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( next->value.empty() && next->name.empty() )
		return 0;
	return next;
}
*/

const TiXmlAttribute* TiXmlAttribute::Previous() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( prev->value.empty() && prev->name.empty() )
		return 0;
	return prev;
}

/*
TiXmlAttribute* TiXmlAttribute::Previous()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( prev->value.empty() && prev->name.empty() )
		return 0;
	return prev;
}
*/

void TiXmlAttribute::Print( FILE* cfile, int /*depth*/, TIXML_STRING* str ) const
{
	TIXML_STRING n, v;

	EncodeString( name, &n );
	EncodeString( value, &v );

	if (value.find ('\"') == TIXML_STRING::npos) {
		if ( cfile ) {
			fprintf (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
		}
		if ( str ) {
			(*str) += n; (*str) += "=\""; (*str) += v; (*str) += "\"";
		}
	}
	else {
		if ( cfile ) {
			fprintf (cfile, "%s='%s'", n.c_str(), v.c_str() );
		}
		if ( str ) {
			(*str) += n; (*str) += "='"; (*str) += v; (*str) += "'";
		}
	}
}


int TiXmlAttribute::QueryIntValue( int* ival ) const
{
	if ( TIXML_SSCANF( value.c_str(), "%d", ival ) == 1 )
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}

int TiXmlAttribute::QueryDoubleValue( double* dval ) const
{
	if ( TIXML_SSCANF( value.c_str(), "%lf", dval ) == 1 )
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}

void TiXmlAttribute::SetIntValue( int _value )
{
	char buf [64];
	#if defined(TIXML_SNPRINTF)
		TIXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
	#else
		sprintf (buf, "%d", _value);
	#endif
	SetValue (buf);
}

void TiXmlAttribute::SetDoubleValue( double _value )
{
	char buf [256];
	#if defined(TIXML_SNPRINTF)
		TIXML_SNPRINTF( buf, sizeof(buf), "%g", _value);
	#else
		sprintf (buf, "%g", _value);
	#endif
	SetValue (buf);
}

int TiXmlAttribute::IntValue() const
{
	return atoi (value.c_str ());
}

double  TiXmlAttribute::DoubleValue() const
{
	return atof (value.c_str ());
}

#ifdef TIXML_USE_IBK_EXTENSIONS
const TiXmlAttribute * TiXmlAttribute::attributeByName(const TiXmlElement * e, const char * const attributeName) {
	if (e == NULL)
		return NULL;
	const TiXmlAttribute* attrib=e->FirstAttribute();
	while (attrib) {
		if (std::string(attrib->Name()) == attributeName) break;
		attrib = attrib->Next();
	}
	return attrib;
}
#endif // TIXML_USE_IBK_EXTENSIONS


TiXmlComment::TiXmlComment( const TiXmlComment& copy ) : TiXmlNode( TiXmlNode::TINYXML_COMMENT )
{
	copy.CopyTo( this );
}


TiXmlComment& TiXmlComment::operator=( const TiXmlComment& base )
{
	Clear();
	base.CopyTo( this );
	return *this;
}


void TiXmlComment::Print( FILE* cfile, int depth ) const
{
	assert( cfile );
#ifdef TIXML_USE_IBK_EXTENSIONS
	// if first character of comment is a line break, print it and start
	// one character later with writing comment
	unsigned int start_idx = 0;
	unsigned int length = value.size();
	if (length == 0)
		return; // empty comment = empty line
	// leading line break, start writing one character later and write a line break immediately.
	if (length > 0 && value[0] == '\n') {
		fprintf( cfile, "\n");
		++start_idx;
		--length;
	}
	// if last character of comment is a line break, rather add the line break
	// after the closing --> than before it.
	bool append_linebreak = false;
	if (length > 0 && value[start_idx+length-1] == '\n') {
		--length; // subtract one from length
		append_linebreak = true; // remember to add line break at the end
	}
	// is there still something left of the comment
	if (length > 0) {
		for ( int i=0; i<depth; i++ ) {
			fprintf( cfile,  "\t" );
		}
		std::string tmp(value.begin()+start_idx, value.begin()+start_idx+length);
		fprintf( cfile, "<!--%s-->", tmp.c_str() );
	}
	if (append_linebreak)
		fprintf( cfile, "\n");
#else // TIXML_USE_IBK_EXTENSIONS
	fprintf( cfile, "<!--%s-->", value.c_str() );
#endif // TIXML_USE_IBK_EXTENSIONS
}


void TiXmlComment::CopyTo( TiXmlComment* target ) const
{
	TiXmlNode::CopyTo( target );
}


bool TiXmlComment::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlComment::Clone() const
{
	TiXmlComment* clone = new TiXmlComment();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}

#ifdef TIXML_USE_IBK_EXTENSIONS
void TiXmlComment::addComment(TiXmlElement * e, const std::string & comment) {
	TiXmlComment * xmlComment = new TiXmlComment();
	xmlComment->SetValue( comment );
	e->LinkEndChild(xmlComment);
}

void TiXmlComment::addSeparatorComment(TiXmlElement * e) {
	TiXmlComment * xmlComment = new TiXmlComment();
	xmlComment->SetValue( '\n' + std::string(100, '~') + '\n');
	e->LinkEndChild(xmlComment);
}
#endif // TIXML_USE_IBK_EXTENSIONS


void TiXmlText::Print( FILE* cfile, int depth ) const
{
	assert( cfile );
	if ( cdata )
	{
		int i;
		fprintf( cfile, "\n" );
		for ( i=0; i<depth; i++ ) {
			fprintf( cfile, "    " );
		}
		fprintf( cfile, "<![CDATA[%s]]>\n", value.c_str() );	// unformatted output
	}
	else
	{
		TIXML_STRING buffer;
		EncodeString( value, &buffer );
		fprintf( cfile, "%s", buffer.c_str() );
	}
}


void TiXmlText::CopyTo( TiXmlText* target ) const
{
	TiXmlNode::CopyTo( target );
	target->cdata = cdata;
}


bool TiXmlText::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlText::Clone() const
{
	TiXmlText* clone = 0;
	clone = new TiXmlText( "" );

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


TiXmlDeclaration::TiXmlDeclaration( const char * _version,
									const char * _encoding,
									const char * _standalone )
	: TiXmlNode( TiXmlNode::TINYXML_DECLARATION )
{
	version = _version;
	encoding = _encoding;
	standalone = _standalone;
}


#ifdef TIXML_USE_STL
TiXmlDeclaration::TiXmlDeclaration(	const std::string& _version,
									const std::string& _encoding,
									const std::string& _standalone )
	: TiXmlNode( TiXmlNode::TINYXML_DECLARATION )
{
	version = _version;
	encoding = _encoding;
	standalone = _standalone;
}
#endif


TiXmlDeclaration::TiXmlDeclaration( const TiXmlDeclaration& copy )
	: TiXmlNode( TiXmlNode::TINYXML_DECLARATION )
{
	copy.CopyTo( this );
}


TiXmlDeclaration& TiXmlDeclaration::operator=( const TiXmlDeclaration& copy )
{
	Clear();
	copy.CopyTo( this );
	return *this;
}


void TiXmlDeclaration::Print( FILE* cfile, int /*depth*/, TIXML_STRING* str ) const
{
	if ( cfile ) fprintf( cfile, "<?xml " );
	if ( str )	 (*str) += "<?xml ";

	if ( !version.empty() ) {
		if ( cfile ) fprintf (cfile, "version=\"%s\" ", version.c_str ());
		if ( str ) { (*str) += "version=\""; (*str) += version; (*str) += "\" "; }
	}
	if ( !encoding.empty() ) {
		if ( cfile ) fprintf (cfile, "encoding=\"%s\" ", encoding.c_str ());
		if ( str ) { (*str) += "encoding=\""; (*str) += encoding; (*str) += "\" "; }
	}
	if ( !standalone.empty() ) {
		if ( cfile ) fprintf (cfile, "standalone=\"%s\" ", standalone.c_str ());
		if ( str ) { (*str) += "standalone=\""; (*str) += standalone; (*str) += "\" "; }
	}
	if ( cfile ) fprintf( cfile, "?>" );
	if ( str )	 (*str) += "?>";
}


void TiXmlDeclaration::CopyTo( TiXmlDeclaration* target ) const
{
	TiXmlNode::CopyTo( target );

	target->version = version;
	target->encoding = encoding;
	target->standalone = standalone;
}


bool TiXmlDeclaration::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlDeclaration::Clone() const
{
	TiXmlDeclaration* clone = new TiXmlDeclaration();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


void TiXmlUnknown::Print( FILE* cfile, int depth ) const
{
	for ( int i=0; i<depth; i++ )
		fprintf( cfile, "    " );
	fprintf( cfile, "<%s>", value.c_str() );
}


void TiXmlUnknown::CopyTo( TiXmlUnknown* target ) const
{
	TiXmlNode::CopyTo( target );
}


bool TiXmlUnknown::Accept( TiXmlVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


TiXmlNode* TiXmlUnknown::Clone() const
{
	TiXmlUnknown* clone = new TiXmlUnknown();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


TiXmlAttributeSet::TiXmlAttributeSet()
{
	sentinel.next = &sentinel;
	sentinel.prev = &sentinel;
}


TiXmlAttributeSet::~TiXmlAttributeSet()
{
	assert( sentinel.next == &sentinel );
	assert( sentinel.prev == &sentinel );
}


void TiXmlAttributeSet::Add( TiXmlAttribute* addMe )
{
	#ifdef TIXML_USE_STL
	assert( !Find( TIXML_STRING( addMe->Name() ) ) );	// Shouldn't be multiply adding to the set.
	#else
	assert( !Find( addMe->Name() ) );	// Shouldn't be multiply adding to the set.
	#endif

	addMe->next = &sentinel;
	addMe->prev = sentinel.prev;

	sentinel.prev->next = addMe;
	sentinel.prev      = addMe;
}

void TiXmlAttributeSet::Remove( TiXmlAttribute* removeMe )
{
	TiXmlAttribute* node;

	for( node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node == removeMe )
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
			node->next = 0;
			node->prev = 0;
			return;
		}
	}
	assert( 0 );		// we tried to remove a non-linked attribute.
}


#ifdef TIXML_USE_STL
TiXmlAttribute* TiXmlAttributeSet::Find( const std::string& name ) const
{
	for( TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node->name == name )
			return node;
	}
	return 0;
}

TiXmlAttribute* TiXmlAttributeSet::FindOrCreate( const std::string& _name )
{
	TiXmlAttribute* attrib = Find( _name );
	if ( !attrib ) {
		attrib = new TiXmlAttribute();
		Add( attrib );
		attrib->SetName( _name );
	}
	return attrib;
}
#endif


TiXmlAttribute* TiXmlAttributeSet::Find( const char* name ) const
{
	for( TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( strcmp( node->name.c_str(), name ) == 0 )
			return node;
	}
	return 0;
}


TiXmlAttribute* TiXmlAttributeSet::FindOrCreate( const char* _name )
{
	TiXmlAttribute* attrib = Find( _name );
	if ( !attrib ) {
		attrib = new TiXmlAttribute();
		Add( attrib );
		attrib->SetName( _name );
	}
	return attrib;
}


#ifdef TIXML_USE_STL
std::istream& operator>> (std::istream & in, TiXmlNode & base)
{
	TIXML_STRING tag;
	tag.reserve( 8 * 1000 );
	base.StreamIn( &in, &tag );

	base.Parse( tag.c_str(), 0, TIXML_DEFAULT_ENCODING );
	return in;
}
#endif


#ifdef TIXML_USE_STL
std::ostream& operator<< (std::ostream & out, const TiXmlNode & base)
{
	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	base.Accept( &printer );
	out << printer.Str();

	return out;
}


std::string& operator<< (std::string& out, const TiXmlNode& base )
{
	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	base.Accept( &printer );
	out.append( printer.Str() );

	return out;
}
#endif


TiXmlHandle TiXmlHandle::FirstChild() const
{
	if ( node )
	{
		TiXmlNode* child = node->FirstChild();
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChild( const char * value ) const
{
	if ( node )
	{
		TiXmlNode* child = node->FirstChild( value );
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChildElement() const
{
	if ( node )
	{
		TiXmlElement* child = node->FirstChildElement();
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::FirstChildElement( const char * value ) const
{
	if ( node )
	{
		TiXmlElement* child = node->FirstChildElement( value );
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::Child( int count ) const
{
	if ( node )
	{
		int i;
		TiXmlNode* child = node->FirstChild();
		for (	i=0;
				child && i<count;
				child = child->NextSibling(), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::Child( const char* value, int count ) const
{
	if ( node )
	{
		int i;
		TiXmlNode* child = node->FirstChild( value );
		for (	i=0;
				child && i<count;
				child = child->NextSibling( value ), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::ChildElement( int count ) const
{
	if ( node )
	{
		int i;
		TiXmlElement* child = node->FirstChildElement();
		for (	i=0;
				child && i<count;
				child = child->NextSiblingElement(), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


TiXmlHandle TiXmlHandle::ChildElement( const char* value, int count ) const
{
	if ( node )
	{
		int i;
		TiXmlElement* child = node->FirstChildElement( value );
		for (	i=0;
				child && i<count;
				child = child->NextSiblingElement( value ), ++i )
		{
			// nothing
		}
		if ( child )
			return TiXmlHandle( child );
	}
	return TiXmlHandle( 0 );
}


bool TiXmlPrinter::VisitEnter( const TiXmlDocument& )
{
	return true;
}

bool TiXmlPrinter::VisitExit( const TiXmlDocument& )
{
	return true;
}

bool TiXmlPrinter::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute )
{
	DoIndent();
	buffer += "<";
	buffer += element.Value();

	for( const TiXmlAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next() )
	{
		buffer += " ";
		attrib->Print( 0, 0, &buffer );
	}

	if ( !element.FirstChild() )
	{
		buffer += " />";
		DoLineBreak();
	}
	else
	{
		buffer += ">";
		if (    element.FirstChild()->ToText()
			  && element.LastChild() == element.FirstChild()
			  && element.FirstChild()->ToText()->CDATA() == false )
		{
			simpleTextPrint = true;
			// no DoLineBreak()!
		}
		else
		{
			DoLineBreak();
		}
	}
	++depth;
	return true;
}


bool TiXmlPrinter::VisitExit( const TiXmlElement& element )
{
	--depth;
	if ( !element.FirstChild() )
	{
		// nothing.
	}
	else
	{
		if ( simpleTextPrint )
		{
			simpleTextPrint = false;
		}
		else
		{
			DoIndent();
		}
		buffer += "</";
		buffer += element.Value();
		buffer += ">";
		DoLineBreak();
	}
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlText& text )
{
	if ( text.CDATA() )
	{
		DoIndent();
		buffer += "<![CDATA[";
		buffer += text.Value();
		buffer += "]]>";
		DoLineBreak();
	}
	else if ( simpleTextPrint )
	{
		TIXML_STRING str;
		TiXmlBase::EncodeString( text.ValueTStr(), &str );
		buffer += str;
	}
	else
	{
		DoIndent();
		TIXML_STRING str;
		TiXmlBase::EncodeString( text.ValueTStr(), &str );
		buffer += str;
		DoLineBreak();
	}
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlDeclaration& declaration )
{
	DoIndent();
	declaration.Print( 0, 0, &buffer );
	DoLineBreak();
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlComment& comment )
{
	DoIndent();
	buffer += "<!--";
	buffer += comment.Value();
	buffer += "-->";
	DoLineBreak();
	return true;
}


bool TiXmlPrinter::Visit( const TiXmlUnknown& unknown )
{
	DoIndent();
	buffer += "<";
	buffer += unknown.Value();
	buffer += ">";
	DoLineBreak();
	return true;
}

