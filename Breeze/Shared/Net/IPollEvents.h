#ifndef __SHARED_NET_IPollEvents_H__
#define __SHARED_NET_IPollEvents_H__

namespace FREEZE_NET
{

	// Virtual interface to be exposed by object that want to be notified
	// about events on file descriptors.

	struct IPollEvents
	{
		virtual ~IPollEvents(void)
		{
		}

		// Called by I/O thread when file descriptor is ready for reading.
		virtual void InEvent(void) = 0;

		// Called by I/O thread when file descriptor is ready for writing.
		virtual void OutEvent(void) = 0;

		// Called when timer expires.
		virtual void TimerEvent(int id) = 0;
	};

}

#endif
