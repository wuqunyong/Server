#ifndef __SHARED_UTILS_GUARD_H__
#define __SHARED_UTILS_GUARD_H__

class Lock;

class Guard
{
public:
	Guard(Lock& lock);
	~Guard(void);

private:
	Guard& operator=(const Guard&);

protected:
	Lock& lock_;
};

#endif
