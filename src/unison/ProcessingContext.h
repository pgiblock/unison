/*
 * ProcessingContext.h
 *
 * Copyright (c) 2010 Paul Giblock <pgib/at/users.sourceforge.net>
 *
 * This file is part of Unison - http://unison.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */



#ifndef PROCESSING_CONTEXT_H
#define PROCESSING_CONTEXT_H

#include "unison/types.h"

namespace Unison {

/** Abstracts parameters needed while rendering nodes.
 *  Instead of relying on plugins to call out to various modules of unison, we
 *  just pass all the needed information along in this handy context. */
class ProcessingContext {
public:
	nframe_t bufferSize() const {
		return 1024;
	}
};

} // Unison

#endif // PROCESSING_CONTEXT_H
