#ifndef DIFFER_H
#define DIFFER_H

#include <vector>
#include <string>
#include <algorithm>

namespace IBK {

enum Operation {
	DifferOpInsert,
	DifferOpRemove,
	DifferOpEqual
};

/*! See https://florian.github.io/diffing */
template <typename T>
class Differ {
public:

	/*! Default constructor */
	Differ() = default;

	/*! Constructor */
	Differ(const std::vector<T> &obj1, const std::vector<T> &obj2);

	/*! Sets a new data set. */
	void setData(const std::vector<T> &obj1, const std::vector<T> &obj2);

	/*! calculates the longest common subsequence (LCS) as a matrix */
	void calculateLCS();

	/*! compares obj1 vs obj2. The result is stored in m_resultObj, conataining the "merged" vector of both objects
		which conatins each element of both vectors. The operation which needs to be done to merge obj2 into obj1 is
		stored in m_resultOperation.
	*/
	void diff();

	/*! the length of the longest common subsequence */
	unsigned int lcsLength();

	/*! Returns merged vector (obj2 merged into obj1). */
	const std::vector<T> & resultObj() const { return m_resultObj; }

	/*! Operation needed for each element. */
	const std::vector<Operation> & resultOperation() const { return m_resultOperation; }

private:
	/*! Stores info if LCS was already calculated for current input data set. */
	bool									m_lcsCalculated = false;

	/*! Cached input data vector 1. */
	std::vector<T>							m_obj1;
	/*! Cached input data vector 2. */
	std::vector<T>							m_obj2;

	/*! Cached LCS matrix. */
	std::vector<std::vector<unsigned int> > m_lcs;

	/*! Merged vector (obj2 merged into obj1). */
	std::vector<T>							m_resultObj;

	/*! Operation needed for each element. */
	std::vector<Operation>					m_resultOperation;

};


template <typename T>
Differ<T>::Differ(const std::vector<T> &obj1, const std::vector<T> &obj2):
	m_lcsCalculated(false),
	m_obj1(obj1),
	m_obj2(obj2)
{
}


template<typename T>
void Differ<T>::setData(const std::vector<T> & obj1, const std::vector<T> & obj2) {
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_lcsCalculated = false;
}


template <typename T>
void Differ<T>::calculateLCS() {
	unsigned int n = m_obj1.size();
	unsigned int m = m_obj2.size();

	// creates a matrix with n+1 rows and m+1 columns, which is needed to compare both objects
	m_lcs = std::vector<std::vector<unsigned int> > (n+1, std::vector<unsigned int>(m+1, 0));

	// we initialized memory and have already:
	//   m_lcs[0][j] = 0;
	//   m_lcs[i][0] = 0;
	for (unsigned int i=1; i<n+1; ++i) {
		for (unsigned int j=1; j<m+1; ++j) {
			if (m_obj1[i - 1] == m_obj2[j - 1])
				m_lcs[i][j] = 1 + m_lcs[i - 1][j - 1];
			else
				m_lcs[i][j] = std::max(m_lcs[i - 1][j], m_lcs[i][j - 1]);
		}
	}
	m_lcsCalculated = true;
}


template <typename T>
unsigned int Differ<T>::lcsLength() {
	if (!m_lcsCalculated)
		calculateLCS();
	return	m_lcs[m_obj1.size()][m_obj2.size()];
}


template <typename T>
void Differ<T>::diff() {
	// first we need to calculate the lcs matrix
	calculateLCS();

	unsigned int i = m_obj1.size();
	unsigned int j = m_obj2.size();

	// reserve capacity in the result vectors
	m_resultObj.clear();
	m_resultObj.reserve(i + j);
	m_resultOperation.clear();
	m_resultOperation.reserve(i + j);

	// LCS[n,m] contains longest common sequence length.
	// Backtrack from largest LCS and construct merged sequence.
	// Because of this, we append the tokens back to front, so at
	// the end we need to reverse the list.

	// loop until both sequences are empty
	while (i != 0 || j != 0) {

		// first sequence is empty, just insert the rest of the second sequence
		if (i==0) {
			m_resultObj.push_back(m_obj2[j-1]);
			m_resultOperation.push_back(DifferOpInsert);
			--j;
		}
		// second sequence is empty, just remove the rest of the second sequence
		else if (j==0) {
			m_resultObj.push_back(m_obj1[i-1]);
			m_resultOperation.push_back(DifferOpRemove);
			--i;
		}
		// both are the same
		else if (m_obj1[i-1] == m_obj2[j-1]){
			m_resultObj.push_back(m_obj1[i-1]);
			m_resultOperation.push_back(DifferOpEqual);
			--i;
			--j;
		}
		// take from second sequence (LCS is longer in that direction)
		else if (m_lcs[i-1][j] <= m_lcs[i][j-1]){
			m_resultObj.push_back(m_obj2[j-1]);
			m_resultOperation.push_back(DifferOpInsert);
			--j;
		}
		else {
			// take from first sequence (LCS is longer in that direction)
			m_resultObj.push_back(m_obj1[i-1]);
			m_resultOperation.push_back(DifferOpRemove);
			--i;
		}
	}

	std::reverse(m_resultObj.begin(), m_resultObj.end());
	std::reverse(m_resultOperation.begin(), m_resultOperation.end());
}

} // namespace IBK


#endif // DIFFER_H
