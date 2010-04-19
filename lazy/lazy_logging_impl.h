/*
 *  lazy_logging_impl.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 22.03.10.
 *  Copyright 2010 Fraunhofer Institut für Software- und Systemtechnik ISST.
 *
 *  This file is part of lazyObject.
 *	
 *	lazyObject is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	
 *	lazyObject is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with lazyObject.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LAZY_LOGGING_IMPL_H_
#define _LAZY_LOGGING_IMPL_H_

#include <lazy.h>
#include <syslog.h>
#include <stdio.h>

#define VERBOSE(...) 
#define DBG(...) // {printf(__VA_ARGS__); printf("\n");}
#define INFO(...) syslog(LOG_INFO, __VA_ARGS__)
#define NOTICE(...) syslog(LOG_NOTICE, __VA_ARGS__)
#define WARNING(...) syslog(LOG_WARNING, __VA_ARGS__)
#define ERR(...) syslog(LOG_ERR, __VA_ARGS__)
#define CRIT(...) syslog(LOG_CRIT, __VA_ARGS__)
#define ALERT(...) syslog(LOG_ALERT, __VA_ARGS__)
#define EMERG(...) syslog(LOG_EMERG, __VA_ARGS__)

#endif // _LAZY_LOGGING_IMPL_H_
