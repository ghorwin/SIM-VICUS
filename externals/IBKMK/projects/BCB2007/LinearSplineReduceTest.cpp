//---------------------------------------------------------------------------

#pragma hdrstop

#include <vector>
#include <iostream>

#include <IBK_LinearSpline.h>
//---------------------------------------------------------------------------

IBK::LinearSpline newXSteps(const IBK::LinearSpline& src, double xSteps, bool average) {
	IBK::LinearSpline res;
	// Step not valid ?
	if( xSteps <= 0)
		return res;
	const std::vector<double>& xvect = src.x();
	const std::vector<double>& yvect = src.y();
	double start = xvect.front();
	double end = xvect.back();
	int count = int((end - start) / xSteps);
	// Step too big?
	if( count < 1 )
		return res;
	double xTest = start + (count-1) * xSteps;
	if( xTest < end)
		++count;

	std::vector<double> xvals;
	std::vector<double> yvals;
	unsigned int istart = 0;
	for( int i=0; i<count; ++i) {
		double currentX = start + i * xSteps;
		xvals.push_back(currentX);
		if( i == 0) {
			yvals.push_back(yvect.front());
		} else {
			if( !average) {
				yvals.push_back(src.value(currentX));
			} else {
				double lastX = xvals[i-1];
				while( xvect[istart] < lastX && istart < xvect.size() - 1)
					++istart;
				unsigned int iend = istart;
				while( xvect[iend] < currentX && iend < xvect.size() - 1)
					++iend;
				--iend;
				double lastY = src.value(lastX);
				double currentY = src.value(currentX);
				if( istart == iend) {
				// No value between
					yvals.push_back((currentY + lastY) / 2.0);
				} else {
					if( xvect[istart] ==  lastX && xvect[iend] == currentX) {
						yvals.push_back((currentY + lastY) / 2.0);
					} else {
						double xt = 0;
						double yt = 0;
						if( xvect[istart] >  lastX) {
							xt += xvect[istart] - lastX;
							yt +=  (lastY + yvect[istart]) / 2.0 * (xvect[istart] - lastX);
						}
						for( unsigned int j=istart; j<iend; ++j) {
							xt += xvect[j+1] - xvect[j];
							yt +=  (yvect[j+1] + yvect[j]) / 2.0 * (xvect[j+1] - xvect[j]);
						}
						if( xvect[iend] <  currentX) {
							xt += currentX - xvect[iend];
							yt +=  (currentY + yvect[iend]) / 2.0 * (currentX - xvect[iend]);
						}
						yt /= xt;
						yvals.push_back(yt);
						istart = iend;
					}
				}
			}
		}
	}
	if( xvals.back() < end) {
		xvals.push_back(end);
		yvals.push_back(yvect.back());
	}
	res.setValues(xvals.begin(), xvals.end(), yvals.begin());
	std::string errstr;
	res.makeSpline(errstr);
	return res;
}

#pragma argsused
int main(int argc, char* argv[])
{
	IBK::LinearSpline src;
	double x[5] = { 0, 1, 2, 3, 4 };
	double y[5] = { 0, 1, 2, 3, 4 };
	src.setValues(&x[0], &x[5], &y[0]);
	std::string errstr;
	if( !src.makeSpline(errstr)) {
		std::cout << "Error!\n" << errstr << "\n";
	} else {
		std::cout << "Spline successfully created\n";
		const std::vector<double>& xtest = src.x();
		const std::vector<double>& ytest = src.y();
		std::cout << "Size: " << xtest.size() << "\n";
		for( unsigned int i=0; i<xtest.size(); ++i) {
			std::cout << xtest[i] << "," << ytest[i] << "; ";
		}
		IBK::LinearSpline res = newXSteps(src, 1.5, true);
		const std::vector<double>& xtest2 = res.x();
		const std::vector<double>& ytest2 = res.y();
		std::cout << "\n\n";
		std::cout << "Size: " << xtest2.size() << "\n";
		for( unsigned int i=0; i<xtest2.size(); ++i) {
			std::cout << xtest2[i] << "," << ytest2[i] << "; ";
		}
	}
	std::cin.get();
	return 0;
}
//---------------------------------------------------------------------------
