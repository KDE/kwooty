/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
 *   xavier.kwooty@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef KWOOTY_EXPORT_H
#define KWOOTY_EXPORT_H
 
// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>
 
#ifndef KWOOTY_EXPORT
# if defined(MAKE_KWOOTY_LIB)
   // We are building this library
#  define KWOOTY_EXPORT KDE_EXPORT
# else
   // We are using this library
#  define KWOOTY_EXPORT KDE_IMPORT
# endif
#endif
 
# ifndef KWOOTY_EXPORT_DEPRECATED
#  define KWOOTY_EXPORT_DEPRECATED KDE_DEPRECATED KWOOTY_EXPORT
# endif
 
#endif