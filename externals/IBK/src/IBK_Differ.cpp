#include "IBK_Differ.h"

#include <algorithm>

namespace IBK {


template<class t>
Differ<t>::Differ()
{
}


template<class t>
Differ<t>::Differ(const std::vector<t> &obj1, const std::vector<t> &obj2):
	m_obj1(obj1),
	m_obj2(obj2)
{
}


template<class t>
void Differ<t>::calculateLCS()
{
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


template<class t>
unsigned int Differ<t>::lcsLength()
{
	if (!m_lcsCalculated)
		calculateLCS();
	return	m_lcs[m_obj1.size()][m_obj2.size()];
}


template<class t>
void Differ<t>::diff()
{
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
