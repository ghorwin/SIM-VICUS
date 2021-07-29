#ifndef DIFFER_H
#define DIFFER_H

#include <vector>
#include <string>
#include <algorithm>

namespace IBK {

/*! See https://florian.github.io/diffing */
template <typename T>
class Differ {
public:
	/*! default constructor */
	Differ() = default;

	/*! constructor */
	Differ(const std::vector<T> &obj1, const std::vector<T> &obj2);

	/*! calculates the longest common subsequence (LCS) as a matrix */
	void calculateLCS();

	/*! compares obj1 vs obj2. The result is stored in m_resultObj, conataining the "merged" vector of both objects
		which conatins each element of both vectors. The operation which needs to be done to merge obj2 into obj1 is
		stored in m_resultOperation.
	*/
	void diff();

	/*! the length of the longest common subsequence */
	unsigned int lcsLength();

	bool									m_lcsCalculated = false;

	std::vector<T>							m_obj1;

	std::vector<T>							m_obj2;

	std::vector<std::vector<unsigned int> > m_lcs;

	std::vector<T >							m_resultObj;

	std::vector<std::string>				m_resultOperation;

};


template <typename T>
Differ<T>::Differ(const std::vector<T> &obj1, const std::vector<T> &obj2):
	m_obj1(obj1),
	m_obj2(obj2)
{
}


template <typename T>
void Differ<T>::calculateLCS() {
	unsigned int n = m_obj1.size();
	unsigned int m = m_obj2.size();

	// creates a matrix with n+1 rows and m+1 columns, which is needed to compare both objects
	m_lcs = std::vector<std::vector<unsigned int> > (n+1, std::vector<unsigned int>(m+1, 0));

	for (unsigned int i=0; i<n+1; ++i){
		for (unsigned int j=0; j<m+1; ++j){

			if (i==0 || j==0)
				m_lcs[i][j] = 0;
			else if (m_obj1[i - 1] == m_obj2[j - 1])
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

	while (i != 0 || j != 0) {

		if (i==0){
			m_resultObj.push_back(m_obj2[j-1]);
			m_resultOperation.push_back("+");
			--j;
		}
		else if (j==0) {
			m_resultObj.push_back(m_obj1[i-1]);
			m_resultOperation.push_back("-");
			--i;
		}

		else if (m_obj1[i-1] == m_obj2[j-1]){
			m_resultObj.push_back(m_obj1[i-1]);
			m_resultOperation.push_back("==");
			--i;
			--j;
		}
		else if (m_lcs[i-1][j] <= m_lcs[i][j-1]){
			m_resultObj.push_back(m_obj2[j-1]);
			m_resultOperation.push_back("+");
			--j;
		}
		else {
			m_resultObj.push_back(m_obj1[i-1]);
			m_resultOperation.push_back("-");
			--i;
		}
	}

	std::reverse(m_resultObj.begin(), m_resultObj.end());
	std::reverse(m_resultOperation.begin(), m_resultOperation.end());
}

} // namespace IBK


#endif // DIFFER_H
