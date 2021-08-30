#ifndef EP_FrameH
#define EP_FrameH

#include <vector>
#include <string>

namespace EP {


/*!
	IDF Frame Class
	CHECK no changes between EP version 8.3 to 9.5.
*/
class Frame
{
public:


	// *** PUBLIC MEMBER FUNCTIONS ***

	Frame(){}

	/*! Read "Frame" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	bool behavesLike(const Frame &other) const;

	/*! Compares this instance with another by value and returns true if they equal. */
	bool operator==(const Frame & other) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const Frame & other)  const { return ! operator==(other); }

	// *** PUBLIC MEMBER VARIABLES ***
	std::string					m_name;
	double						m_widthFrame;			// m
	double						m_widthDivider;			// m
	double						m_conductanceFrame;		// W/m2K (exclude air films)
	double						m_conductanceDivider;	// W/m2K (exclude air films)
};
}

#endif // EP_FrameH
