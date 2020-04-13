/*
* Copyright (c) 2011 Erin Catto http://box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <Box2D/Common/b2Settings.h>

/// Timer for profiling. This has platform specific code and may
/// not work on every platform.
class b2Timer
{
public:

	/// Constructor
	b2Timer();

	/// Reset the timer.
	void Reset();

	/// Get the time since construction or the last reset.
	float32 GetMilliseconds() const;

private:

#if defined(_WIN32)
	float64 m_start;
	static float64 s_invFrequency;
#elif defined(__linux__) || defined (__APPLE__)
	unsigned long m_start_sec;
	unsigned long m_start_msec;
#endif
};
