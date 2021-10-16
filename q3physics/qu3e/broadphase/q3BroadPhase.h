//--------------------------------------------------------------------------------------------------
/**
@file	q3BroadPhase.h

@author	Randy Gaul
@date	10/10/2014

	Copyright (c) 2014 Randy Gaul http://www.randygaul.net
	Modified Nick Bradley (2021)

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	  1. The origin of this software must not be misrepresented; you must not
	     claim that you wrote the original software. If you use this software
	     in a product, an acknowledgment in the product documentation would be
	     appreciated but is not required.
	  2. Altered source versions must be plainly marked as such, and must not
	     be misrepresented as being the original software.
	  3. This notice may not be removed or altered from any source distribution.
*/
//--------------------------------------------------------------------------------------------------

#ifndef Q3BROADPHASE_H
#define Q3BROADPHASE_H

#include "../common/q3Types.h"
#include "q3DynamicAABBTree.h"
#include "../common/q3Memory.h"

//--------------------------------------------------------------------------------------------------
// q3BroadPhase
//--------------------------------------------------------------------------------------------------
class q3ContactManager;
struct q3Box;
struct q3Transform;
struct q3AABB;
class q3Scene;

struct q3ContactPair
{
	i32 A;
	i32 B;
};

class q3BroadPhase {
protected:
	q3ContactManager *m_manager;
public:
	
	q3BroadPhase( q3ContactManager *manager );
	q3BroadPhase( q3Scene *scene );
	virtual ~q3BroadPhase();
	
	virtual void InsertBox( q3Box *shape, const q3AABB& aabb ) = 0;
	virtual void RemoveBox( const q3Box *shape ) = 0;

	// Generates the contact list. All previous contacts are returned to the allocator
	// before generation occurs.
	virtual void UpdatePairs( void ) = 0;

	virtual void Update( i32 id, const q3AABB& aabb ) = 0;

	virtual bool TestOverlap( q3Box* A, q3Box* B ) const = 0;
	
	friend class q3Scene;
};

class q3DefaultBroadPhase : public q3BroadPhase
{
public:
	q3DefaultBroadPhase( q3ContactManager *manager );
	~q3DefaultBroadPhase( );

	virtual void InsertBox( q3Box *shape, const q3AABB& aabb );
	virtual void RemoveBox( const q3Box *shape );

	// Generates the contact list. All previous contacts are returned to the allocator
	// before generation occurs.
	virtual void UpdatePairs( void );

	virtual void Update( i32 id, const q3AABB& aabb );

	virtual bool TestOverlap( q3Box* A, q3Box* B ) const;

private:

	q3ContactPair* m_pairBuffer;
	i32 m_pairCount;
	i32 m_pairCapacity;

	i32* m_moveBuffer;
	i32 m_moveCount;
	i32 m_moveCapacity;

	q3DynamicAABBTree m_tree;
	i32 m_currentIndex;

	void BufferMove( i32 id );
	bool TreeCallBack( i32 index );

	friend class q3DynamicAABBTree;
	friend class q3Scene;
};

inline bool q3DefaultBroadPhase::TreeCallBack( i32 index )
{
	// Cannot collide with self
	if ( index == m_currentIndex )
		return true;

	if ( m_pairCount == m_pairCapacity )
	{
		q3ContactPair* oldBuffer = m_pairBuffer;
		m_pairCapacity *= 2;
		m_pairBuffer = (q3ContactPair*)q3Alloc( m_pairCapacity * sizeof( q3ContactPair ) );
		memcpy( m_pairBuffer, oldBuffer, m_pairCount * sizeof( q3ContactPair ) );
		q3Free( oldBuffer );
	}

	i32 iA = q3Min( index, m_currentIndex );
	i32 iB = q3Max( index, m_currentIndex );

	m_pairBuffer[ m_pairCount ].A = iA;
	m_pairBuffer[ m_pairCount ].B = iB;
	++m_pairCount;

	return true;
}

#endif // Q3BROADPHASE_H
