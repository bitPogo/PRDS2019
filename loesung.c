/*
* =====================================================================================
* 
* Filename:  loesung.c
* 
* Description:  Abgabe zum Programmier Praktikum SoSe2019
* 
* Version:  1.b
* Created:  23.08.2019 06:20:22
* Revision:  none
* Compiler:  gcc
* 
* Author:  Matthias Geisler (https://meta.wikimedia.org/wiki/User:Matthias_Geisler_(WMDE)), geislemx@informatik.hu-berlin.de
* Organization:  private
*
* =====================================================================================
*/
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>
#include <signal.h>

#ifndef LF
#define LF 10
#endif
#ifndef CR
#define CR 13
#endif
#define TO_LOWER 32
#ifndef EXIT_ERR
#define EXIT_ERR 23
#endif
#ifndef EXIT_ILE
#define EXIT_ILE 1
#endif

/*=============================Globals( Pseudo Head )==========================*/
size_t min( size_t A, size_t B );
void errorAndOut( const char* Message );
char* substring( char* Source, size_t From, size_t Length, bool* Error );
bool startsWith( const char* Str1, const char* Str2 );
// bool endsWith( const char* Str1, const char* Str2 );
char* makeEmptyString( bool* Error );
char* makeStrCopy( const char* ToCopy, bool* Error );
char* cat( char* Str1, char* Str2, bool* Error );
/*===============================PatricaTrie Defs==============================*/
typedef struct PNode {
	struct PNode* parent;
	struct PNode** children;
	unsigned short sizeOfChildren;
	char* value;
	char* key;
	// bool root;
} PNode;

#ifdef DEBUGPR
char* getKey( const PNode* Self, bool* Error );
#endif
size_t longestPrefix( const PNode* Self, const char* Key );
/* const PNode* _findByKey( const PNode* Self, const char* Key, bool MatchExact ); */
const PNode* _findByKey( const PNode* Self, const char* MyKey, const char* Key, bool MatchExact );
void printKeys( const PNode* Self, bool* Error );

PNode* makeNewPNode( bool* Error )
{
	PNode* NewNode;
	
	NewNode = (PNode* ) malloc( sizeof(PNode)*1 );
	if( NULL == NewNode )
	{
		*Error = true;
		return NULL;
	}

	NewNode->parent = NULL;
	NewNode->children = NULL;
	NewNode->sizeOfChildren = 0;
	
	NewNode->key = makeEmptyString( Error );
	if( NULL == NewNode->key )
	{
		free( NewNode );
		return NULL;
	}
	
	NewNode->value = NULL;
	// NewNode->root = false;

	return NewNode;
}

/*
void destroyPNode( PNode* Node, bool Recursive )
{
	unsigned short Index;

	if( true == Recursive && 0 < Node->sizeOfChildren )
	{
		for( Index = 0; Node->sizeOfChildren>Index; Index++ )
		{
			destroyPNode( Node->children[ Index ], Recursive );
		}
	}

	free( Node->children );
	free( Node->key );

	if( NULL != Node->value )
	{
		free( Node->value );
	}

	free( Node );
}*/

void _freeNode( PNode* Node )
{
	free( Node->children );
	free( Node->key );
	
	if( NULL != Node->value )
	{   
		free( Node->value );
	}
	
	free( Node );
}

void _iterativeRevemovel( PNode* Root )
{
	PNode* CurrentNode;
	PNode* Parent;

	while( 0 < Root->sizeOfChildren )
	{
		Parent = Root;
		CurrentNode = Root->children[ Root->sizeOfChildren-1 ];
		
		while( 0 < CurrentNode->sizeOfChildren )
		{
			Parent = CurrentNode;
			CurrentNode = Parent->children[ Parent->sizeOfChildren-1 ];
		}
		
		Parent->sizeOfChildren--;
		_freeNode( CurrentNode );
	}
}

void destroyPNode( PNode* Node, bool Purge )
{
	if( true == Purge && 0 < Node->sizeOfChildren )
	{
		_iterativeRevemovel( Node );
	}
	_freeNode( Node );
}
/*------------------------------------Basement-------------------------------*/
char* _getKey( const PNode* Self )
{
	return Self->key;
}

#ifdef DEBUGPR
char* _getPrefix( const PNode* Self, bool* Error )
{
	char* Return;
	char* Tmp;
	PNode* Parent;

	if( NULL == Self->parent )
	{
		return makeEmptyString( Error );
	}
	else
	{
		Parent = Self->parent;
		Return = makeEmptyString( Error );
		if( NULL == Return )
		{
			return NULL;
		}

		while( NULL != Parent )
		{
			Tmp = cat( Parent->key, Return, Error );
			free( Return );

			if( NULL == Tmp )
			{
				return NULL;
			}
			
			Return = Tmp;
			Parent = Parent->parent;
		}

		return Return;
	}
}
#endif

size_t longestPrefix( const PNode* Self, const char* Key )
{
	register size_t To;
	register size_t Index;
	char* MyKey;

	MyKey = _getKey( Self );

	To = min( 
			strlen( Key ), 
			strlen( MyKey ) 
	);

	for( Index = 0; To > Index; Index++ )
	{
		if( Key[ Index ] != MyKey[ Index ] )
		{
			return Index;
		}
	}
	
	return To;
}

void setValue( PNode* Self, const char* Value, bool* Error )
{
	if( NULL != Value )
	{
		if( NULL != Self->value )
		{
			free( Self->value );
		}
		Self->value = makeStrCopy( Value, Error );
	}
}

char* getValue( const PNode* Self, bool* Error )
{
	return makeStrCopy( Self->value, Error );
}

void setKey( PNode* Self, const char* Key, bool PreventCopy, bool* Error )
{
	free( Self->key );
	if( true == PreventCopy )
	{
		Self->key = (char *) Key;	
	}
	else
	{
		Self->key = makeStrCopy( Key, Error );
	}
}
/*----------------------------------Reading----------------------------------*/
#ifdef DEBUGPR
char* getKey( const PNode* Self, bool* Error )
{
	char* Prefix;
	char* Return;
	
	if( NULL == Self->parent )
	{
		return makeEmptyString( Error );
	}
	else
	{
		Prefix = _getPrefix( Self, Error );
		if( NULL == Prefix )
		{
			return NULL;
		}

		Return = cat( Prefix, _getKey( Self ), Error );

		free( Prefix );
		if( NULL == Return )
		{	
			return NULL;
		}

		return Return;
	}
}
#endif

short __searchForChild( const PNode* Self, char Key )
{
	short Start, End, Middle;

	if( 0 == Self->sizeOfChildren || _getKey( Self->children[ 0 ] )[0] > Key )
	{
		return -1;
	}

	Start = 0;
	End = Self->sizeOfChildren-1;
	
	if ( _getKey( Self->children[ End ] )[0] < Key )
	{
		return -1;
	}
	
	while ( Start <= End )
	{
		Middle = ( ( Start + End ) >> 1 );
		if( Middle > Self->sizeOfChildren ) 
		{
			return -1;
		}

		if ( Key > _getKey( Self->children[ Middle ])[0] )
		{
			Start = Middle + 1;
		}
		else if ( Key < _getKey( Self->children[ Middle ])[0] )
		{
			End = Middle - 1;
		}
		else
		{
			return Middle;
		}
	}

	return -1;
}

/*
const PNode* _commonPrefix( const PNode* Node, const char* NodeKey, const char* Key , bool MatchExact )
{   
	char* NewStart;
	if( true == MatchExact )
	{
		if( 0 == strcmp( NodeKey, Key ) )
		{
			return Node;
		}
	}
	else
	{
		if( true == startsWith( NodeKey, Key ) )
		{   
			return Node;
		}
	}

	if( true == startsWith( Key, NodeKey ) )
	{   
		NewStart = (char *)Key + ( strlen( NodeKey ) * sizeof( char ) );
		return _findByKey( Node, NewStart, MatchExact );
	}
	else
	{   
		return NULL;
	}
}

const PNode* _findByKey( const PNode* Self, const char* Key, bool MatchExact )
{
	short Index;
	const char* MyKey;
	char* StrStart;
	const PNode* CurrentNode;

	CurrentNode = Self;
	StrStart = (char* )Key;

	Index = __searchForChild( CurrentNode, StrStart[ 0 ] );
	if( -1 == Index )
	{
		return NULL;
	}
	
	MyKey = _getKey( CurrentNode->children[ Index ] );	
	return _commonPrefix(
			CurrentNode->children[ Index ],
			MyKey,
			StrStart,
			MatchExact
	);
}
*/
const PNode* _findByKey( const PNode* Self, const char* MyKey, const char* Key, bool MatchExact )
{
	short Index;
	char* StrStart;
	const PNode* CurrentNode;
	
	CurrentNode = Self;
	StrStart = (char* )Key;
	do 
	{
		if( true == MatchExact )
		{
			if( 0 == strcmp( MyKey, StrStart ) )
			{
				return CurrentNode;
			}
		}
		else
		{
			if( true == startsWith( MyKey, StrStart ) )
			{
				return CurrentNode;
			}
		}

		//if( true == startsWith( StrStart, MyKey ) && false == CurrentNode->root )
		if( true == startsWith( StrStart, MyKey ) && NULL != CurrentNode->parent )
		{
			StrStart = (char *)StrStart + ( strlen( MyKey ) * sizeof( char ) );
		}
	
		Index = __searchForChild( CurrentNode, StrStart[ 0 ] );
		if( -1 == Index )
		{
			return NULL;
		}

		CurrentNode = CurrentNode->children[ Index ];
		MyKey = _getKey( CurrentNode );
	}
	while( true );
}

const PNode* findByKey( 
	const PNode* Self, 
	const char* Key,
	bool* Error,
	bool IsPrefixed,
	bool MatchExact
)
{
	const char* MyKey;
	const PNode* Return;
	bool MakeFree = false;

#ifdef DEBUGPR
	if( true == IsPrefixed )
	{
		MakeFree = true;
		MyKey = getKey( Self, Error );
		if( true == *Error )
		{
			return NULL;
		}
	}
	else
	{
		MyKey = _getKey( Self );
	}
#endif
	if( true == IsPrefixed ){}
	if( true == *Error ){}
	MyKey = _getKey( Self );

	/*Return = _commonPrefix(
		Self,
		MyKey,
		Key,
		MatchExact
	);*/
	Return = _findByKey(
		Self,
		MyKey,
		Key,
		MatchExact
	);


	if( true == MakeFree )
	{
		free( (char *) MyKey );
	}

	return Return;
}

const PNode* findEndPointByKey(
	const PNode* Self,
	const char* Key,
	bool* Error,
	bool IsPrefixed,
	bool MatchExact
) 
{
	const PNode* Node;

	Node = findByKey( Self, Key, Error, IsPrefixed, MatchExact );
	if( NULL != Node )
	{
		if( NULL != Node->value )
		{
			return Node;
		}
	}

	return NULL;
}

char* findValueByKey(
	const PNode* Self,
	const char* Key,
	bool* Error,
	bool IsPrefixed,
	bool MatchExact
) {
	const PNode* Tmp;

	Tmp = findEndPointByKey(
		Self,
		Key,
		Error,
		IsPrefixed,
		MatchExact
	);

	if( NULL != Tmp )
	{
		return getValue( Tmp, Error );
	}
	else
	{
		return NULL;
	}
}

short _insertPosition( const PNode* Self, char Key )
{
	register short Start;
	register short End;
	register short Middle;

	if( 0 == Self->sizeOfChildren || _getKey( Self->children[ 0 ] )[0] > Key )
	{
		return -1;
	}

	Start = 0;
	End = Self->sizeOfChildren-1;

	if ( _getKey( Self->children[ End ] )[0] < Key )
	{
		return -( Self->sizeOfChildren + 1 );
	}
	
	while ( Start <= End )
	{
		Middle = ( ( Start + End ) >> 1 );
		if ( Middle > Self->sizeOfChildren ) 
		{
			return -( Start + 1 );
		}

		if ( Key > _getKey( Self->children[ Middle ] )[0] )
		{
			Start = Middle + 1;
		}
		else if ( Key < _getKey( Self->children[ Middle ] )[0] )
		{
			End = Middle - 1;
		}
		else
		{
			return Middle;
		}
	}

	return -( Start + 1 );	
}

void __setChildParent( PNode* Parent, PNode* Child, unsigned short Index )
{
	Child->parent = Parent; 
	Parent->children[ Index ] = Child;
}

PNode* __appendChild( PNode* Where, PNode* NewChild, register unsigned short PlaceToBe, bool* Error )
{
	PNode** NewChildren;
	register unsigned short Index;
	register unsigned short Index2;
	size_t NewSize;

	if( 0 == Where->sizeOfChildren )
	{   
		NewChildren = (PNode** ) calloc( sizeof( PNode* ), 1 );
		if( NULL == NewChildren )
		{
			*Error = true;
			return NULL;
		}
	}
	else
	{   
		NewSize = Where->sizeOfChildren+1;
		NewChildren = (PNode** ) calloc( sizeof( PNode* ), NewSize );// this might a bit slow, but better for the memory
		if( NULL == NewChildren )
		{
			*Error = true;
			return NULL;
		}

		memcpy( NewChildren, Where->children, Where->sizeOfChildren*sizeof( PNode* ) );
	
		/* Splicing */

		NewChildren[ PlaceToBe ] = NewChild;
		
		for( Index = PlaceToBe+1, Index2 = PlaceToBe; NewSize > Index; Index++, Index2++ )
		{
			NewChildren[ Index ] = Where->children[ Index2 ];
		}
	}

	free( Where->children );
	
	Where->sizeOfChildren++;
	Where->children = NewChildren;
	__setChildParent( ( PNode* ) Where, ( PNode* )NewChild, PlaceToBe );
	
	return NewChild;
}

PNode* _insertChild(
	PNode* Where,
	const char* Key,
	const char* Value,
	register unsigned short Index,	
	bool* Error
)
{
	PNode* NewChild;
	PNode* Return;
	
	NewChild = makeNewPNode( Error );
	if( NULL == NewChild )
	{
		*Error = true;
		return NULL;
	}

	setKey( NewChild, Key, false, Error );
	if( true == *Error )
	{
		*Error = true;
		destroyPNode( NewChild, false );
		return NULL;
	}

	setValue( NewChild, Value, Error );
	if( true == *Error )
	{
		*Error = true;
		destroyPNode( NewChild, false );
		return NULL;
	}

	Return = __appendChild( Where, NewChild, Index, Error );
	if( NULL == Return )
	{
		destroyPNode( NewChild, false );
		return NULL;
	}

	return Return;
}

PNode* _appendChild( 
	PNode* Self, 
	PNode* Child, 
	bool Force, 
	bool* Error 
)
{
	short Index;

	Index = _insertPosition( Self, Child->key[ 0 ] );

	if( -1 < Index )
	{
		if( true == Force )
		{
			Self->children[ Index ] = Child;
			return Child;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		Index = -( Index + 1 );	
		return __appendChild(
			Self,
			Child,
			Index,
			Error
		);
	}
}
/*
PNode* _insert( 
	PNode* Self, 
	const char* OrgKey,
	const char* Value,
	bool Force,
	bool* Error 
)
{
	size_t PrefixLength, KeyLength, InsertKeyLength;
	short Index;
	PNode* NewParent;
	PNode* Return;
	PNode* Tmp;
	char* Key;
	char* NewKey;
	char* CommonKey;
	
	Key = (char*) OrgKey;
	
	PrefixLength = longestPrefix( Self, Key );
	KeyLength = strlen( _getKey( Self ) );
	InsertKeyLength = strlen( Key );

	if( 0 == PrefixLength )
	{
        if( 0 == Self->sizeOfChildren )
		{
			return _insertChild( Self, Key, Value, 0, Error );
		}

		Index = _insertPosition( Self, Key[ 0 ] );
		
		if( -1 < Index )
		{
			return _insert( Self->children[ Index ], Key, Value, Force, Error );
		}
		else
		{
			Index = -( Index + 1 ); 
			return _insertChild( (PNode* )Self, Key, Value, Index, Error );
		}
	}
	else if( PrefixLength == InsertKeyLength && PrefixLength == KeyLength )
	{
		if( NULL == Self->value )
		{
			setValue( Self, Value, Error );
			if( true == *Error )
			{
				return NULL;
			}

			return Self;
		}

		if( true == Force )
		{
			free( Self->value );
			Self->value = NULL;
			setValue( (PNode* )Self, Value, Error );
			if( true == *Error )
			{
				return NULL;
			}
			
			return Self;
		}

		return NULL;
	}
	else if( PrefixLength == KeyLength )
	{
		Key = Key+( PrefixLength * sizeof( char ) );

		if( 0 == Self->sizeOfChildren )
		{
			return _insertChild( Self, Key, Value, 0, Error );
		}

		Index = _insertPosition( Self, Key[ 0 ] );
		
		if( 0 <= Index )
		{
			return _insert( Self->children[ Index ], Key, Value, Force, Error );
		}
		else
		{
			Index = -( Index + 1 );
			return _insertChild( (PNode* )Self, Key, Value, Index, Error );
		}
	}
	else if( PrefixLength == InsertKeyLength )
	{
		Index = __searchForChild( Self->parent, _getKey( Self )[ 0 ] );
		NewKey = substring( _getKey( Self ), PrefixLength, strlen( _getKey( Self ) ), Error );
		if( NULL == NewKey )
		{
			return NULL;
		}

		NewParent = makeNewPNode( Error );
		if( true == *Error )
		{
			free( NewKey );
			return NULL;
		}

		setKey( ( PNode* )NewParent, Key, false, Error );
		if( true == *Error )
		{
			free( NewKey );
			destroyPNode( NewParent, false );
			return NULL;
		}

		setValue( (PNode* )NewParent, Value, Error );
		if( true == *Error )
		{
			free( NewKey );
			destroyPNode( NewParent, false );
			return NULL;
		}
		
		setKey( NewParent, Key, false, Error );
		if( true == *Error )
		{
			free( NewKey );
			destroyPNode( NewParent, false );
			return NULL;
		}

		setKey( Self, NewKey, true, Error );
		
		Tmp = Self->parent;
		
		Return = _appendChild( 
				NewParent, 
				Self, 
				false,
				Error
		);
		if( NULL == Return )
		{
			destroyPNode( NewParent, false );
			return NULL;
		}
		
		
		__setChildParent( Tmp, NewParent, Index  );
		NewParent->parent = Tmp;
		return NewParent;
	}
	else
	{
		CommonKey = (char *) _getKey( Self );
		CommonKey = substring( CommonKey, 0, PrefixLength, Error );
		if( NULL == CommonKey )
		{
			return NULL;
		}
	
		NewKey = substring( Key, PrefixLength, strlen( Key ), Error );
		if( NULL == NewKey )
		{
			free( CommonKey );
			return NULL;
		}

		NewParent = (PNode* )_insert( 
				Self,
				CommonKey,
				NULL,
				false,
				Error
		);

		free( CommonKey );
		
		if( NULL == NewParent )
		{
			free( NewKey );
			return NULL;
		}

		Index = _insertPosition( NewParent, NewKey[ 0 ] );
		Index = -( Index + 1 );
		Return = _insertChild( NewParent, NewKey, Value, Index, Error ); 
		free( NewKey );

		return Return;
	}
}*/

PNode* _insert(
	PNode* Self,
	const char* OrgKey,
	const char* Value,
	bool Force,
	bool* Error
)
{
	size_t PrefixLength, KeyLength, InsertKeyLength;
	short Index;
	PNode* NewParent;
	PNode* Return;
	PNode* Tmp;
	PNode* CurrentNode;
	char* Key;
	char* NewKey;
	char* CommonKey;
	
	Key = (char*) OrgKey;
	CurrentNode = (PNode *) Self;
	
	while( true )
	{
		PrefixLength = longestPrefix( CurrentNode, Key );
		KeyLength = strlen( _getKey( CurrentNode ) );
		InsertKeyLength = strlen( Key );
		
		if( 0 == PrefixLength )
		{
			if( 0 == CurrentNode->sizeOfChildren )
			{
				return _insertChild( CurrentNode, Key, Value, 0, Error );
			}
			
			Index = _insertPosition( CurrentNode, Key[ 0 ] );
			
			if( -1 < Index )
			{
				CurrentNode = CurrentNode->children[ Index ];
				continue;
			}
			else
			{
				Index = -( Index + 1 );
				return _insertChild( CurrentNode, Key, Value, Index, Error );
			}
        }
        else if( PrefixLength == InsertKeyLength && PrefixLength == KeyLength )
        {
			if( NULL == CurrentNode->value )
			{
				setValue( CurrentNode, Value, Error );
				if( true == *Error )
				{
					return NULL;
				}
				
				return CurrentNode;
			}
			
			if( true == Force )
			{
				free( CurrentNode->value );
				CurrentNode->value = NULL;
				setValue( CurrentNode, Value, Error );
				if( true == *Error )
				{
					return NULL;
				}
				
				return CurrentNode;
			}
			
			return NULL;
        }
		else if( PrefixLength == KeyLength )
		{
			Key = Key+( PrefixLength * sizeof( char ) );
			
			if( 0 == CurrentNode->sizeOfChildren )
			{
				return _insertChild( CurrentNode, Key, Value, 0, Error );
			}
			
			Index = _insertPosition( CurrentNode, Key[ 0 ] );
			if( 0 <= Index )
			{
				CurrentNode = CurrentNode->children[ Index ];
				continue;
			}
			else
			{
				Index = -( Index + 1 );
				return _insertChild( CurrentNode, Key, Value, Index, Error );
			}
		}
		else if( PrefixLength == InsertKeyLength )
        {
			Index = __searchForChild( CurrentNode->parent, _getKey( CurrentNode )[ 0 ] );
			NewKey = substring( _getKey( CurrentNode ), PrefixLength, strlen( _getKey( CurrentNode ) ), Error );
			
			if( NULL == NewKey )
			{
				return NULL;
			}
			
			NewParent = makeNewPNode( Error );
			if( true == *Error )
			{
				free( NewKey );
				return NULL;
			}
			
			setKey( ( PNode* )NewParent, Key, false, Error );
			if( true == *Error )
			{
				free( NewKey );
				destroyPNode( NewParent, false );
				return NULL;
			}
			
			setValue( (PNode* )NewParent, Value, Error );
			if( true == *Error )
			{
				free( NewKey );
				destroyPNode( NewParent, false );
				return NULL;
			}
			
			setKey( CurrentNode, NewKey, true, Error );// note: no error check needed -> we just reuse the given pointer
			Tmp = CurrentNode->parent;
			
			Return = _appendChild(
					NewParent,
					CurrentNode,
					false,
					Error
			);
			
			if( NULL == Return )
			{
				destroyPNode( NewParent, false );
				return NULL;
			}
			
			__setChildParent( Tmp, NewParent, Index  );
			NewParent->parent = Tmp;
			return NewParent;
        }
		else
		{
			CommonKey = (char *) _getKey( CurrentNode );
			CommonKey = substring( CommonKey, 0, PrefixLength, Error );
			if( NULL == CommonKey )
			{
				return NULL;
			}
			
			NewKey = substring( Key, PrefixLength, strlen( Key ), Error );
			if( NULL == NewKey )
			{
				free( CommonKey );
				return NULL;
			}
			
			NewParent = (PNode* )_insert(
					CurrentNode,
					CommonKey,
					NULL,
					false,
					Error
			);
			free( CommonKey );
			
			if( NULL == NewParent )
			{
				free( NewKey );
				return NULL;
			}
			
			Index = _insertPosition( NewParent, NewKey[ 0 ] );
			Index = -( Index + 1 );
			Return = _insertChild( NewParent, NewKey, Value, Index, Error );
			free( NewKey );
			return Return;
		}
	}
}

const PNode* insert(
	PNode* Self,
	const char* Key,
	const char* Value,
	bool Force,
	bool* Error
)
{
	short Index;
	Index = _insertPosition( Self, Key[ 0 ] );
	if( -1 < Index )
	{
		return _insert( Self->children[ Index ], Key, Value, Force, Error );
	}
	else
	{
		Index = -( Index + 1 );
		return _insertChild( (PNode* )Self, Key, Value, Index, Error );
	}
}
/*----------------------------------Debug------------------------------------*/
#ifdef DEBUGPR
void printKeys( const PNode* Self, bool* Error )
{
	unsigned short Index;
	char* Key;
	
	if( NULL != Self->value )
	{
		Key = getKey( Self, Error );
		if( true == *Error )
		{
			return;
		}

		printf( "%s:%s\n", Key, Self->value );
		free( Key );
	}
	
	for( Index = 0; Self->sizeOfChildren>Index; Index++ )
	{
		printKeys( Self->children[ Index ], Error );
	}
}
#endif
/*===================================Utils===================================*/
 char* makeStrCopy( const char* ToCopy, bool* Error )
{
	size_t Len;
	char* Dolly;
	
	Len = strlen( ToCopy );
	Len++;//nullbit
	
	Dolly = (char* ) calloc( sizeof( char ), Len );
	if( NULL == Dolly )
	{
		*Error = true;
		return NULL;
	}
	
	strcpy( Dolly, ToCopy );
	return Dolly;
}


char* makeEmptyString( bool* Error ) 
{
	char* EmptyString;
	
	EmptyString = (char *) malloc( sizeof(char) );
	if( NULL == EmptyString )
	{
		*Error = true;
		return NULL;
	}
	
	EmptyString[ 0 ] = '\0';
	return EmptyString;
}
/* Predefine globals */
PNode* Dictionary;
FILE* CurrentSource;
/**
 * Prints a error-message to stderr and quits the programm
 * @param Message | const char* | the message
 */
void errorAndOut( const char* Message )
{
	fclose( CurrentSource );
	destroyPNode( Dictionary, true );
	fprintf( stderr, "\n%s\n", Message);
	fflush( stderr );
	exit( EXIT_ERR );
}
/**
 * Returns a substring of given String
 * @param Source | char* | the input string
 * @param From | int | the startposition of the subset
 * @param Length | int | Length of the subset
 * @return | char* | the computed subset
 */
char* substring( char* Source, size_t From, size_t Length, bool* Error )
{
    char* Return;
    register size_t Index1;
	register size_t Index2;
	register size_t SourceLength;

    if( NULL == Source )
    {
        return NULL;
    }

	SourceLength = strlen( Source );
	if( Length > SourceLength || From > SourceLength || From > Length )
	{
		return NULL;
	}

    if( 0 == From && SourceLength == Length)
    {
       return makeStrCopy( Source, Error );
    }

    Return = (char *) malloc( ( Length + 1 )*sizeof(char) );
    if( NULL == Return )
    {
    	*Error = true;
		return NULL;
	}
	
	for( Index1=From, Index2=0; Length>Index1; Index1++, Index2++)
	{
		Return[ Index2 ] = Source[ Index1 ];
	}
	
	Return[ Index2 ] = '\0';

    return Return;
}

bool startsWith( const char* Str1, const char* Str2 )
{
	size_t Len1, Len2;
	Len1 = strlen( Str1 );
	Len2 = strlen( Str2 );
	
	if( Len2 > Len1 )
	{
		return false;
	}
	
	if( 0 != strncmp( Str1, Str2, Len2 ) )
	{
		return false;
	}
	
	return true;
}

/*
bool endsWith( const char* Str1, const char* Str2 )
{
	size_t Len1, Len2, Offset, Index;
	Len1 = strlen( Str1 );
	Len2 = strlen( Str2 );
	
	if( Len1 == Len2 )
	{
		return startsWith( Str1, Str2 );
	}
	
	if( Len2 > Len1 )
	{
		return false;
	}
	
	Offset = Len1-Len2;
	for( Index = 0; Index < Len1; Index++)
	{
		if( Str1[ Offset+Index ] != Str2[ Index ] )
		{
			return false;
		}
	}

	return true;
}
*/
char* cat( char* Str1, char* Str2, bool* Error )
{
	size_t PayloadSize;
	char* Return;

	PayloadSize = strlen( Str1 ) + strlen( Str2 );
	
	Return = (char *) malloc( ( PayloadSize+1 )*sizeof(char) );
	if( NULL == Return )
	{
		*Error = true;
		return NULL;
	}
	
	strcpy( Return, Str1 );
	strcat( Return, Str2 );
	Return[ PayloadSize ] = '\0';
	return Return;
}

size_t min( size_t A, size_t B )
{
	return A > B ? A : B;
}
/*===================================Flow=====================================*/
/*=====================================Globals=================================*/
/* make sure we devide between:
 * 0 Wenn die Eingaben formal gültig sind und jedes Wort im Text übersetzt werden kann.
 * 1 Wenn die Eingaben formal gültig sind, im Text aber Wörter auftreten, die nicht übersetzt werden können.
*/
bool EarlyExit;
short Return;
/*----------------------------------DICT--------------------------------------*/
void readInputFile( char* Path );
wint_t nextChar( FILE* Source );
void parseDict( FILE* Source );
bool buildDict( const char* Key, const char* Value, bool* MemError );
void evilFromStdin();
bool pushToBuffer( char** Buffer, char InputChar );

void makeEarlyExit( int Signal )
{
	if( true == EarlyExit )
	{
		exit( Signal );
	}

	if( SIGSEGV != Signal )
	{
		EarlyExit = true;
		return;
	}
}

int main( int ArgC, char* Arguments[] ) 
{
	bool Error;
	Return = 0;

	setlocale( LC_ALL, "" );
	if ( 1 == ArgC ) 
	{
		errorAndOut( "Too few arguments provided." );
	}

	if( 2 < ArgC )
	{
		errorAndOut( "To many arguments provided." );
	}

	Error = false;
	signal( SIGINT, makeEarlyExit );
	signal( SIGILL, makeEarlyExit );
	signal( SIGABRT, makeEarlyExit );
	signal( SIGTERM, makeEarlyExit );
	
	Dictionary = makeNewPNode( &Error );
	// Dictionary->root = true;
	if( true == Error )
	{
		errorAndOut( "Something seems wrong with the memory. Over and out." );
	}

#ifdef DEBUGPR
	printf( "Start preproc\n" );
#endif

	readInputFile( Arguments[ 1 ] );

#ifdef DEBUGPR
	bool Internal;
	Internal = false;
	printKeys( Dictionary, &Internal );

	printf( "Start main proc\n" );
#endif

#ifdef DEBUGPR
	printf( "done....\n" );
#endif
	
	evilFromStdin();
	printf( "\n" );

	destroyPNode( Dictionary, true );
	if( 0 != Return )
	{
		exit( EXIT_ILE );
	}
	return 0;
}

/*----------------------------------DICT--------------------------------------*/
void readInputFile( char* Path )
{
	FILE* FilePointer = fopen( Path, "r" );
	char ErrorMsg[ 120 ];
	
	if( !FilePointer || ferror( FilePointer ) )
	{
		snprintf( ErrorMsg, 120, "Cannot open given file (%s).", Path );
		errorAndOut( ErrorMsg );
    }

	CurrentSource = FilePointer;
	parseDict( FilePointer );

	fclose( FilePointer );
}

wint_t nextChar( FILE* Source )
{
	return fgetwc( Source );
}

void parseDict( FILE* Source )
{
	bool Mode, DoneFirstChar;
	bool Error, MemError;
	register wint_t CurrentChar;
	register wint_t LookAHead;
	char* Key;
	char* Value;
	unsigned long Line;
	char ErrorMsg[ 150 ];
	
	Mode = 0;
	DoneFirstChar = 0;
	Line = 1;

	Error = false;
	MemError = false;

	Key = makeEmptyString( &Error );
	if( true == Error )
	{
		errorAndOut( "Something went wrong with the memory, jim." );
	}

	Value = makeEmptyString( &Error );
	if( true == Error )
	{
		free( Key );
		errorAndOut( "I cannot get enough mem...." );
	}

	LookAHead = nextChar( Source );

	/* Die leere Eingabe ist eine gültige Eingabe. Das gilt sowohl für das Wörterbuch als auch für den Übersetzungstext.*/
	if( WEOF == LookAHead )
	{
		free( Key );
		free( Value );
		return;
	}

	while( WEOF != LookAHead )
	{
		CurrentChar = LookAHead;
		LookAHead = nextChar( Source );
		if( ferror( Source ) )
		{
			free( Key );
			free( Value );
			errorAndOut( "I/O error when reading." );
		}

		/* CR -> LF && CR LF -> LF: Der Einfachheit halber dürfen Sie neben einem einzelnen Linefeed (LF)
		 * auch ein carriage return(CR) oder ein „CR LF“ als Zeilenumbruch im Wörterbuch akzeptieren.
		 */
		if( CR == CurrentChar )
		{
			if( LF == LookAHead )
			{
				continue;
			}

			CurrentChar = LF;
		}

		/* Colon Rule
		 * Eine Zeile enthält nicht genau einen Doppelpunkt.
		 * Der Doppelpunkt in einer Zeile ist das erste oder das letzte Zeichen
		 */
		if( ':' == CurrentChar )
		{
			if( 
				1 == Mode
			||
				LF == LookAHead
			||
				CR == LookAHead
			||
				WEOF == LookAHead
			||
				0 == DoneFirstChar
			)
			{
				free( Key );
				free( Value );
				snprintf( 
						ErrorMsg, 
						150, 
						"The given dictionary is incorrectly formed( Line %lu ).\n\t-Mode: %i\n\t-LR: %i\n\t-CR: %i\n\t-EOF: %i\n\t-DoneFirstChar: %i\n\t-given char: '%c'", 
						Line,
						Mode, 
						LF == LookAHead, 
						CR == LookAHead, 
						WEOF == LookAHead,
						DoneFirstChar,
						(char )CurrentChar
				);
				errorAndOut( ErrorMsg );
			}

			Mode = 1;
			continue;
		}

		/* LineFeed Rule 
		 * Eine Zeile enthält nicht genau einen Doppelpunkt
		 */
		if( LF == CurrentChar )
		{
			if( 0 == Mode )
			{
				free( Key );
				free( Value );
				snprintf(
						ErrorMsg,
						120,
						"The given dictionary is incorrectly formed - unexpected EOL on Line %lu.",
						Line
				);
				errorAndOut( ErrorMsg );
			}

			Error = buildDict( Key, Value, &MemError );
			
			free( Key );
			free( Value );

			if( true == MemError )
			{
				snprintf(
					ErrorMsg,
					120,
					"You really trie hard to throw fireworks...memory r.i.p. on Line %lu.",
					Line
				);
				errorAndOut( ErrorMsg );
			}

			if( true == Error )
			{
				snprintf( ErrorMsg, 120, "Repeated entry detected on Line %lu.", Line );
				errorAndOut( ErrorMsg );
			}

			if( true == EarlyExit )
			{
				errorAndOut( "Interrupted - closing savely." );
			}


			DoneFirstChar = 0;
			Mode = 0;
			Line++;

			Key = makeEmptyString( &Error );
			if( true == Error )
			{
				errorAndOut( "There is something wrong with, the memory, Sir!" );
			}
			
			Value = makeEmptyString( &Error );
			if( true == Error )
			{
				free( Key );
				errorAndOut( "There is no space left." );
			}
			
			continue;
		}

		/* Valid Chars
		 * Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf.
		 */
		if( 'a' > CurrentChar || 'z' < CurrentChar )
		{
			free( Key );
			free( Value );
			snprintf(
					ErrorMsg,
					120,
					"The given dictionary is incorrectly formed - illegal token with value %u on Line %lu.",
					CurrentChar,
					Line
			);
			errorAndOut( ErrorMsg );
		}

		DoneFirstChar = 1;

		/*
		 * a bit slow, could be improved by using buffers + I do not like realloc
		 */
		if( 0 == Mode )
		{
			if( false == pushToBuffer( &Key, (char )CurrentChar ) )
			{
				 free( Key );
				 free( Value );
				 errorAndOut( "Memory breach...." );
			}	
		}
		else
		{
			if( false == pushToBuffer( &Value, (char )CurrentChar ) )
			{
				free( Key );
				free( Value );
				errorAndOut( "Just annother memory error message." );
			}
		}
	}

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	free( Key );
	free( Value );

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	if( 0 != DoneFirstChar )
	{
		errorAndOut( "The given dictionary is incorrectly formed - Missing linefeed before end of file." );
	}
}

bool buildDict( const char* Key, const char* Value, bool* MemError )
{
	return NULL == insert( 
		Dictionary,
		Key,
		Value,
		false,
		MemError
	);
}

bool pushToBuffer( char** Buffer, char InputChar )
{
	size_t Size, OldSize;
	char* Tmp;

	OldSize = strlen( *Buffer );
	Size = strlen( *Buffer )+2;//new char + \0
	Tmp = (char*) calloc( Size, sizeof( char ) );
	if( NULL == Tmp )
	{
		return false;
	}

	memcpy( Tmp, *Buffer, OldSize );
	Tmp[ OldSize ] = InputChar;
	Tmp[ Size-1 ] = '\0';
	free( *Buffer );
	*Buffer = Tmp;
	return true;
}

void evilFromStdin()
{
	/* Note this could be improved, by using partial keys and free them after node switch */
	char* CaseInSensitive;
	char* CaseSentive;
	/* --- */
	char* Translation;
	register wint_t CurrentChar;
	bool UpperCase;
	bool Error;

	Error = false;
	CurrentSource = stdin;

	CaseSentive = makeEmptyString( &Error );
	if( true == Error )
	{
		errorAndOut( "Memory fail...." );
	}

	CaseInSensitive = makeEmptyString( &Error );
	if( true == Error )
	{
		errorAndOut( "And we are out - Memory fail...." );
	}


	UpperCase = false;
	CurrentChar = nextChar( stdin );
	
	while( WEOF != CurrentChar )
	{
		if( ferror( stdin ) )
		{
			free( CaseInSensitive );
			free( CaseSentive );
			errorAndOut( "I/O error when reading from stdin." );
		}

		if( true == EarlyExit )
		{
			free( CaseInSensitive );
			free( CaseSentive );
			errorAndOut( "Interrupted - closing savely." );
		}

		/* Der eingegebene Text (stdin) sei genau dann gültig, wenn er ausschließlich die Zeichen 10
		 * (line feed) sowie 32–126(inklusive) enthält.
		 */
		if( 10 != CurrentChar && ( ' ' > CurrentChar || '~' < CurrentChar ) )
		{
			free( CaseInSensitive );
			free( CaseSentive );
			errorAndOut( "Illeagl input from stdin in detected." );
		}

		/* Der erste Buchstabe eines übersetzten
		 * Wortes sei genau dann ein Großbuchstabe, wenn der erste Buchstabe im Ursprungstext groß
		 * ist; alle übrigen Buchstaben des übersetzten Wortes sind immer klein.
		 */
		if( 'A' <= CurrentChar && 'Z' >= CurrentChar )
		{
			UpperCase = true&!(0<strlen(CaseInSensitive));
			if( false == pushToBuffer( &CaseInSensitive, (char )( CurrentChar + TO_LOWER ) ) )
			{
				free( CaseSentive );
				free( CaseInSensitive );
				errorAndOut( "Ohhhhh memory fail......" );
			}

			if( false == pushToBuffer( &CaseSentive, (char )( CurrentChar ) ) )
			{
				free( CaseSentive );
				free( CaseInSensitive );
				errorAndOut( "And the memory is out......" );
			}
			
			CurrentChar = nextChar( stdin );
			continue;
		}
		
		if( 'a' > CurrentChar || 'z' < CurrentChar )// && ( 'A' > CurrentChar || 'Z' < CurrentChar ) )
		{
			if( 0 < strlen( CaseInSensitive ) )
			{
				Translation = findValueByKey( 
					Dictionary,
					CaseInSensitive,
					&Error,
					false,
					true
				);

				if( true == Error )
				{
					free( CaseSentive );
					free( CaseInSensitive );
					errorAndOut( "Why you do this...there is nothing left..." );
				}

				if( NULL == Translation )
				{
					printf( "<%s>", CaseSentive );
					Return = 1;
				}
				else
				{
					if( true == UpperCase )
					{
						Translation[ 0 ] -= TO_LOWER;
					}

					printf( "%s", Translation );
				}

				free( Translation );
				free( CaseSentive );
				free( CaseInSensitive );
				
				CaseSentive = makeEmptyString( &Error );
				if( true == Error )
				{
					errorAndOut( "Nope, there is no memory left...I am sorry" );
				}

				CaseInSensitive = makeEmptyString( &Error );
				if( true == Error )
				{
					free( CaseSentive );
					errorAndOut( "End of the line bro, there is no memory left..." );
				}
			}

			printf( "%c", (char )CurrentChar );
			UpperCase = false;
			CurrentChar = nextChar( stdin );
			continue;
		}
		
		if( false == pushToBuffer( &CaseInSensitive, (char )( CurrentChar ) ) )
		{
			free( CaseSentive );
			free( CaseInSensitive );
			errorAndOut( "Oh my - no no memory......" );
		}
		
		if( false == pushToBuffer( &CaseSentive, (char )( CurrentChar ) ) )
		{
			free( CaseSentive );
			free( CaseInSensitive );
			errorAndOut( "Wow you hit the last memory fail message possible." );
		}

		CurrentChar = nextChar( stdin );
	}

	free( CaseSentive );
	free( CaseInSensitive );
	
	fclose( stdin );
}
