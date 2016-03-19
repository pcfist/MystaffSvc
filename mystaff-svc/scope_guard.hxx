/**
 * @file scope_guard.hxx
 * Simple scope guard object for RAII pattern implementation.
 * @author pcfist	@date 2013.07.31
 */
#ifndef _scope_guard_hxx_
#define _scope_guard_hxx_

#include <utility>

template <typename Ty>
class scope_guard
{
public:
	scope_guard(Ty &&fn) : fn_(fn), enabled_(true) { }
	scope_guard(scope_guard<Ty> &&other)
	  : fn_(std::move(other.fn_)), enabled_(other.enabled_) {
		other.enabled_ = false;
	}

	~scope_guard() {
		if (enabled_)
			fn_();
	}

	void dismiss() {
		enabled_ = false;
	}

	bool operator=(bool enabled) {
		enabled_ = enabled;
		return enabled;
	}


private:
	Ty fn_;
	bool enabled_;

	scope_guard(const scope_guard &) /*= delete*/;
	scope_guard& operator=(const scope_guard &) /*= delete*/;
};


template <class Ty> inline
scope_guard<Ty> make_scope_guard(Ty &&fn) {
	return scope_guard<Ty>(std::forward<Ty>(fn));
}

#endif // _scope_guard_hxx_
