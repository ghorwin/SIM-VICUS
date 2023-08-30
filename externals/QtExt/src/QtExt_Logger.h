#ifndef QtExt_LoggerH
#define QtExt_LoggerH

#include <fstream>

namespace QtExt {


class Logger {
public:
	static Logger& instance() {
		static Logger log;
		return log;
	}

	inline void set(const std::string& filename) {
		if(filename.empty()) {
			if(m_opened)
				m_out.close();
			m_opened = false;
		}
		else {
			m_out.open(filename);
			m_opened = m_out.is_open();
		}
	}

	template<typename T>
	inline Logger& operator<<(const T& msg) {
		if(m_opened) {
			m_out << msg << std::endl;
		}
		return *this;
	}

private:
	Logger() = default;

	bool m_opened = false;
	std::ofstream m_out;

};

} // end namespace

#endif // QtExt_LoggerH
