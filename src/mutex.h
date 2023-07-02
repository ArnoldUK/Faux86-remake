/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers
  
  Contributions and Updates (c)2023 Curtis aka ArnoldUK

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#ifdef _WIN32
    #include <Windows.h>
    #include <process.h>
    #define MutexLock(mutex) EnterCriticalSection(&mutex)
    #define MutexUnlock(mutex) LeaveCriticalSection(&mutex)
	#define InitMutex(mutex) InitializeCriticalSection (&mutex);
	typedef CRITICAL_SECTION Mutex;
#elif defined(LINUX)
    #include <pthread.h>
    #define MutexLock(mutex) pthread_mutex_lock(&mutex)
    #define MutexUnlock(mutex) pthread_mutex_unlock(&mutex)
	#define InitMutex(mutex) mutex = PTHREAD_MUTEX_INITIALIZER
	typedef pthread_mutex_t Mutex;	
#else
	#define MutexLock(mutex)
	#define MutexUnlock(mutex)
	#define InitMutex(mutex)
	typedef int Mutex;
#endif
