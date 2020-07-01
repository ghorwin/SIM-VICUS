/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

    Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
    All rights reserved.

    This file is part of the IBK Library.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


    This library contains derivative work based on other open-source libraries.
    See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_ptr_listH
#define IBK_ptr_listH

#include "IBK_configuration.h"


#include "IBK_algorithm.h"

#include <list>
#include <stdexcept>
#include <string>

#include "IBK_FormatString.h"
#include "IBK_Exception.h"

namespace IBK {

/*! This class is a base class for lists of pointers to objects.
    The class ptr_list encapsulates a STL-Container, which contains the
    addresses to dynamically created objects and provides special features.
    Unlike STL containers of pointers the objects pointed to in the ptr_list
    are destructed upon destruction of the list itself.
    New pointers are added to the list via the member function add().
    There are several functions to remove pointers with important differences:
    ptr_remove(), remove() and release().
*/
template <class T>
class ptr_list {
    typedef std::list<T*>                         list_type;
  public:
    typedef T*                                    			value_type;       ///< Data type of _values_ pointed to from the list.
    typedef typename list_type::iterator          			iterator;         ///< Iterator type to elements of the list.
    typedef typename list_type::const_iterator    			const_iterator;   ///< Constant iterator type to elements of the list.
    typedef typename list_type::reverse_iterator          	reverse_iterator;         ///< Reverse iterator type to elements of the list.
    typedef typename list_type::const_reverse_iterator    	const_reverse_iterator;   ///< Constant reverse iterator type to elements of the list.
    typedef typename list_type::reference         			reference;        ///< Reference type to elements of the list.
    typedef typename list_type::const_reference   			const_reference;  ///< Constant reference type to elements of the list.

    /*! Destructor, releases memory of the objects in the list. */
    virtual ~ptr_list();

    /*! Adds a new pointer to the list.
        You may not add the same pointer (address) twice. Otherwise upon
        destruction of the list it would lead to destructing the same object
        twice, which causes an exception.
        If you still try to add a pointer twice, an exception is thrown.
        @see release, remove, ptr_remove
    */
    void add(T* ptr);

    /*! Releases the memory of the objects that are pointed to by the elements in
        the range 'first' to 'last'.
        For all pointers in the given range delete is called and thus the memory of
        the objects pointed to is released. The pointers are set to NULL afterwards.
        @see add, remove, ptr_remove
    */
    void release(iterator first, iterator last);
    /*! Releases the memory of the object that is pointed to by the iterator 'it'.
        The object to which the iterator 'it' points is deleted and the pointer is
        set to NULL afterwards.
        @see add, remove, ptr_remove
    */
    void release(iterator it) { iterator tmp(it); release(it, ++tmp); }

    /*! Removes the pointers in the range 'first' to 'last' from the list while releasing
        the memory of the objects at the same time.
        This function both releases the memory and removes the pointers from the list.
        @see add, release, ptr_remove
    */
    void remove(iterator first, iterator last);
    /*! Removes the pointer at the iterator position 'it'.
        This function both releases the memory of the object and removes the pointer from the list.
        @see add, release, ptr_remove
    */
    void remove(iterator it) { iterator tmp(it); remove(it, ++tmp); }

    /*! Removes only the pointers in the range from the list, the memory is NOT released.
        Be careful when using the function. If you remove a pointer from a list without
        having the means to release the memory of the object later, you will get a
        resource leak.
        @see add, release, remove
    */
    void ptr_remove(iterator start, iterator finish);
    /*! Removes only the pointer from the list, the memory is NOT released.
        Be careful when using the function. If you remove a pointer from a list without
        having the means to release the memory of the object later, you will get a
        resource leak.
        @see add, release, remove
    */
    void ptr_remove(iterator it) { iterator tmp(it); ptr_remove(it, ++tmp); }

    /*! Releases all objects and removes all pointers from the list.
        This is a convenience function and is the same as calling remove(begin(), end()).
    */
    void clear();

    /*! Sorts the list according to the sort rule defined in the predicate object. */
    template<typename P>
    void sort(P Predicate) { list_.sort(Predicate); }

    /*! Iterator to the first pointer in the list. */
    iterator begin()                { return list_.begin(); }
    /*! Iterator to the first pointer in the list (read only). */
    const_iterator begin() const    { return list_.begin(); }
    /*! Iterator to the first pointer in the list. */
    reverse_iterator rbegin()       { return list_.rbegin(); }
    /*! Iterator to the first pointer in the list (read only). */
    const_reverse_iterator rbegin() const    { return list_.rbegin(); }
    /*! Iterator to one after the last pointer in the list. */
    iterator end()                  { return list_.end(); }
    /*! Iterator to one after the last pointer in the list (read only). */
    const_iterator end() const      { return list_.end(); }

    /*! Returns a reference to the last element in the list. */
    reference         back() { iterator last = list_.end(); return *(--last); }
    /*! Returns a reference to the last element in the list (read only). */
    const_reference   back() const { const_iterator last = list_.end(); return *(--last); }

#ifndef IBK_DEBUG
    /*! Returns the pointer at index 'i'.
        In debug mode the function throws an IBK_exception
        if the index is out of range.
    */
    T* operator[](unsigned int i) { return *advance_iterator( list_.begin(), i);	}
    /*! Returns the pointer at index 'i' (read-only pointer).
        In debug mode the function throws an IBK_exception
        if the index is out of range.
    */
    T* operator[](unsigned int i) const { return *advance_iterator( list_.begin(), i);	}

#else // IBK_DEBUG
    T* operator[](unsigned int i);
    T* operator[](unsigned int i) const;
#endif // IBK_DEBUG

    /*! Returns the pointer at index 'i'. */
    T* at(unsigned int i);
    /*! Returns the pointer at index 'i' (read-only pointer). */
    T* at(unsigned int i) const;


    /*! Returns the size of the list. */
    unsigned int size() const       { return (unsigned int)list_.size(); }
    /*! Returns 'true' of the list does not contain any pointers. */
    bool         empty() const      { return list_.empty(); }

protected:
    list_type	list_;	///< The data member that stores the values
};

template <class T>
ptr_list<T>::~ptr_list() {
    remove(list_.begin(), list_.end());
}

template <class T>
void ptr_list<T>::add(T* ptr) {
    iterator endit( list_.end() );
    const_iterator it( std::find(list_.begin(), list_.end(), ptr) );
    if (ptr == NULL || it==endit)
        list_.push_back(ptr);
    else throw IBK::Exception("Pointer already exists in the list.","[ptr_list<T>::add]");
}

template <class T>
void ptr_list<T>::release(iterator first, iterator last) {
    while (first!=last) {
        delete *first;
        *first++ = NULL;
    }
}

template <class T>
void ptr_list<T>::remove(iterator first, iterator last) {
    iterator it = first;
    while (it!=last)
        delete *it++;
    list_.erase(first, last);
}

template <class T>
void ptr_list<T>::ptr_remove(iterator first, iterator last) {
    list_.erase(first, last);
}

template <class T>
void ptr_list<T>::clear() {
    remove(list_.begin(), list_.end());
}

#ifdef IBK_DEBUG

template <class T>
T* ptr_list<T>::operator[](unsigned int i) {
    if (i>=list_.size())
        throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].").arg(i).arg(0).arg(size()-1),
            "[ptr_list<T>::operator[] ]");
    iterator it( list_.begin() );
    while (i>0) ++it, --i;
    return *it;
}

template <class T>
T* ptr_list<T>::operator[](unsigned int i) const {
    if (i>=list_.size())
        throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].")
            .arg(i).arg(0).arg(static_cast<unsigned int>(size()-1)),
            "[ptr_list<T>::operator[] ]");
    const_iterator it( list_.begin() );
    while (i>0) ++it, --i;
    return *it;
}

#endif // IBK_DEBUG

template <class T>
T* ptr_list<T>::at(unsigned int i) {
    if (i>=list_.size())
        throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].")
            .arg(i).arg(0).arg(static_cast<unsigned int>(size()-1)),
            "[ptr_list<T>::at]");
    iterator it( list_.begin() );
    while (i>0) ++it, --i;
    return *it;
}

template <class T>
T* ptr_list<T>::at(unsigned int i) const {
    if (i>=list_.size())
        throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].") .arg(i) .arg(0) .arg(size()-1),
            "[ptr_list<T>::at]");
    const_iterator it( list_.begin() );
    while (i>0) ++it, --i;
    return *it;
}


}  // namespace IBK

/*! \file IBK_ptr_list.h
    \brief Contains the declaration of the class template ptr_list.

    \example ptr_list.cpp
    This small example demonstrates the general use of the class IBK::ptr_list.
*/

#endif // IBK_ptr_listH
