#ifndef OutputSchedulerH
#define OutputSchedulerH

namespace SOLFRA {

/*! Computes suitable output time points. */
class OutputScheduler {
public:

	/*! Virtual destructor. */
	virtual ~OutputScheduler() {}

	/*! Returns next output time point after a given time t.
		\param t Some given time point.
		\return Returns output time point that lies beyond time point t.
	*/
	virtual double nextOutputTime(double t);


};

} // namespace SOLFRA


#endif // OutputSchedulerH

