#ifndef __SHARED_UTILS_NONCOPYABLE_H__
#define __SHARED_UTILS_NONCOPYABLE_H__

class Noncopyable
{
protected:
	Noncopyable()
	{
	}
	~Noncopyable()
	{
	}
private:
	// emphasize the following members are private
	Noncopyable(const Noncopyable&);
	const Noncopyable& operator=(const Noncopyable&);
};

#endif
