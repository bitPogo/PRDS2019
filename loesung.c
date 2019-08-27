/*
* =====================================================================================
* 
* Filename:  loesung.c
* 
* Description:  Abgabe zum Programmier Praktikum SoSe2019
* 
* Version:  1.0
* Created:  23.08.2019 06:20:22
* Revision:  none
* Compiler:  gcc
* 
* Author:  Matthias Geisler (https://meta.wikimedia.org/wiki/User:Matthias_Geisler_(WMDE)), geislemx@informatik.hu-berlin.de
* Organization:  private
*
* =====================================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef LF
#define LF 10
#endif
#ifndef CR
#define CR 13
#endif
#define TO_LOWER 32

/*=====================================Types===================================*/
/* This is supposed to live in a seperate file */
typedef struct StringBuffer
{
	char* string;
	size_t length;
} StringBuffer;
/*=============================Globals( Pseudo Head )==========================*/
char* Trace; //DEBUG
char* Trace2; //DEBUG
char* Trace3; //DEBUG
char* Trace4; //DEBUG

unsigned long min( unsigned long A, unsigned long B );
void errorAndOut( const char* Message );
char* substring( char* Source, size_t From, size_t Length, bool* ErrorFlag );
bool startsWith( const char* Str1, const char* Str2 );
bool endsWith( const char* Str1, const char* Str2 );
char* makeEmptyString( bool* ErrorFlag );
char* makeStrCopy( const char* ToCopy, bool* ErrorFlag );
/*===============================PatricaTrie Defs==============================*/
typedef struct PNode {
	struct PNode* parent;
	struct PNode** children;
	unsigned short sizeOfChildren;
	char* value;
	char* key;
	bool root;//Debug perpuse
} PNode;

char* getKey( const PNode* Self, bool* ErrorFlag );
size_t longestPrefix( const PNode* Self, const char* Key );
const PNode* _findByKey( const PNode* Self, const char* Key, bool MatchExact );
void printKeys( const PNode* Self, bool* ErrorFlag );

PNode* makeNewPNode( bool* ErrorFlag )
{
	PNode* NewNode;
	
	NewNode = (PNode* ) malloc( sizeof(PNode)*1 );
	if( NULL == NewNode )
	{
		*ErrorFlag = true;
		return NULL;
	}

	NewNode->parent = NULL;
	NewNode->children = NULL;
	NewNode->sizeOfChildren = 0;
	
	NewNode->key = makeEmptyString( ErrorFlag );
	if( NULL == NewNode->key )
	{
		free( NewNode );
		return NULL;
	}
	
	NewNode->value = NULL;
	NewNode->root = false;

	return NewNode;
}

// TODO Stack req
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

	if( 0 < Node->sizeOfChildren )
	{
		free( Node->children );
	}

	free( Node->key );

	if( NULL != Node->value )
	{
		free( Node->value );
	}

	free( Node );
}

/*------------------------------------Basement-------------------------------*/
char* _getKey( const PNode* Self )
{
	return Self->key;
}

char* _getPrefix( const PNode* Self, bool* ErrorFlag )
{
	if( NULL == Self->parent )
	{
		return makeEmptyString( ErrorFlag );
	}
	else
	{
		return (char *) getKey( Self->parent, ErrorFlag );
	}
}

size_t longestPrefix( const PNode* Self, const char* Key )
{
	size_t To, Index;
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

void setValue( PNode* Self, const char* Value, bool* ErrorFlag )
{
	if( NULL != Value )
	{
		if( NULL != Self->value )
		{
			free( Self->value );
		}
		Self->value = makeStrCopy( Value, ErrorFlag );
	}
}

char* getValue( const PNode* Self, bool* ErrorFlag )
{
	return makeStrCopy( Self->value, ErrorFlag );
}

char* die() {
	char* d = "asdaequqejsadfpuqoerqnerqeqjqheqwe sosdfsfqierqo imasklfn";
	free( d );
	free( d );
	d = NULL;
	return d;
}

void setKey( PNode* Self, const char* Key, bool PreventCopy, bool* ErrorFlag )
{
	free( Self->key );
	if( true == PreventCopy )
	{
		Self->key = (char *) Key;	
	}
	else
	{
		Self->key = makeStrCopy( Key, ErrorFlag );
	}
}
/*----------------------------------Reading----------------------------------*/
char* getKey( const PNode* Self, bool* ErrorFlag )
{
	char* Tmp;
	char* Return;
	size_t PayloadSize;
	
	if( NULL == Self->parent )
	{
		return makeEmptyString( ErrorFlag );
	}
	else
	{
		Tmp = _getPrefix( Self, ErrorFlag );
		if( NULL == Tmp )
		{
			return NULL;
		}

		PayloadSize = strlen( Tmp ) + strlen( Self->key );
		
		Return = (char *) malloc( ( PayloadSize+1 )*sizeof(char) );
		if( NULL == Return )
		{
			*ErrorFlag = true;
			return NULL;
		}

		strcpy( Return, Tmp );
		strcat( Return, Self->key );
		Return[ PayloadSize ] = '\0';
	
		free( Tmp );
		return Return;
	}
}

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
	
	if( true == startsWith( StrStart, MyKey ) )
	{
		StrStart = (char *)StrStart + ( strlen( MyKey ) * sizeof( char ) );
		return _commonPrefix(
				CurrentNode->children[ Index ],
				_getKey( CurrentNode->children[ Index ] ),
				StrStart,
				MatchExact
		);
	}
	else
	{
		return NULL;
	}
}

const PNode* findByKey( 
	const PNode* Self, 
	const char* Key,
	bool* ErrorFlag,
	bool IsPrefixed,
	bool MatchExact
)
{
	const char* MyKey;
	const PNode* Return;
	bool MakeFree = false;
	
	if( true == IsPrefixed )
	{
		MakeFree = true;
		MyKey = getKey( Self, ErrorFlag );
		if( true == *ErrorFlag )
		{
			return NULL;
		}
	}
	else
	{
		MyKey = _getKey( Self );
	}
	
	Return = _commonPrefix(
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
	bool* ErrorFlag,
	bool IsPrefixed,
	bool MatchExact
) 
{
	const PNode* Node;

	Node = findByKey( Self, Key, ErrorFlag, IsPrefixed, MatchExact );
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
	bool* ErrorFlag,
	bool IsPrefixed,
	bool MatchExact
) {
	const PNode* Tmp;

	Tmp = findEndPointByKey(
		Self,
		Key,
		ErrorFlag,
		IsPrefixed,
		MatchExact
	);

	if( NULL != Tmp )
	{
		return getValue( Tmp, ErrorFlag );
	}
	else
	{
		return NULL;
	}
}

short _insertPosition( const PNode* Self, char Key )
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

PNode* __appendChild( PNode* Where, PNode* NewChild, unsigned short Index, bool* ErrorFlag )
{
	PNode** NewChildren;
	unsigned short Index2, Index3;
	size_t NewSize;

	if( 0 == Where->sizeOfChildren )
	{   
		NewChildren = (PNode** ) calloc( sizeof( PNode* ), 1 );
		if( NULL == NewChildren )
		{
			*ErrorFlag = true;
			return NULL;
		}
	}
	else
	{   
		NewSize = Where->sizeOfChildren+1;
		NewChildren = (PNode** ) calloc( sizeof( PNode* ), NewSize );// this might a bit slow, but better for the memory
	
		if( NULL == NewChildren )
		{
			*ErrorFlag = true;
			return NULL;
		}
	
		/* Splicing */
		for( Index2 = 0; Index > Index2; Index2++ )
		{
			NewChildren[ Index2 ] = Where->children[ Index2 ];
		}

		NewChildren[ Index ] = NewChild;
		
		for( Index2 = Index+1, Index3 = Index; NewSize > Index2; Index3++, Index2++ )
		{
			NewChildren[ Index2 ] = Where->children[ Index3 ];
		}
	}

	free( Where->children );
	
	Where->sizeOfChildren++;
	Where->children = NewChildren;
	__setChildParent( ( PNode* ) Where, ( PNode* )NewChild, Index );
	
	return NewChild;
}

PNode* _insertChild(
	PNode* Where,
	const char* Key,
	const char* Value,
	unsigned short Index,	
	bool* ErrorFlag
)
{
	PNode* NewChild;
	PNode* Return;
	
	NewChild = makeNewPNode( ErrorFlag );
	if( NULL == NewChild )
	{
		*ErrorFlag = true;
		return NULL;
	}

	setKey( NewChild, Key, false, ErrorFlag );
	if( true == *ErrorFlag )
	{
		*ErrorFlag = true;
		destroyPNode( NewChild, false );
		return NULL;
	}

	setValue( NewChild, Value, ErrorFlag );
	if( true == *ErrorFlag )
	{
		*ErrorFlag = true;
		destroyPNode( NewChild, false );
		return NULL;
	}

	Return = __appendChild( Where, NewChild, Index, ErrorFlag );
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
	bool* ErrorFlag 
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
			ErrorFlag
		);
	}
}

PNode* _insert( 
	PNode* Self, 
	const char* OrgKey,
	const char* Value,
	bool Force,
	bool* ErrorFlag 
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
			return _insertChild( Self, Key, Value, 0, ErrorFlag );
		}

		Index = _insertPosition( Self, Key[ 0 ] );
		
		if( -1 < Index )
		{
			return _insert( Self->children[ Index ], Key, Value, Force, ErrorFlag );
		}
		else
		{
			Index = -( Index + 1 ); 
			return _insertChild( (PNode* )Self, Key, Value, Index, ErrorFlag );
		}
	}
	else if( PrefixLength == InsertKeyLength && PrefixLength == KeyLength )
	{
		if( NULL == Self->value )
		{
			setValue( Self, Value, ErrorFlag );
			if( true == *ErrorFlag )
			{
				return NULL;
			}

			return Self;
		}

		if( true == Force )
		{
			free( Self->value );
			Self->value = NULL;
			setValue( (PNode* )Self, Value, ErrorFlag );
			if( true == *ErrorFlag )
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
			return _insertChild( Self, Key, Value, 0, ErrorFlag );
		}

		Index = _insertPosition( Self, Key[ 0 ] );
		
		if( 0 <= Index )
		{
			return _insert( Self->children[ Index ], Key, Value, Force, ErrorFlag );
		}
		else
		{
			Index = -( Index + 1 );
			return _insertChild( (PNode* )Self, Key, Value, Index, ErrorFlag );
		}
	}
	else if( PrefixLength == InsertKeyLength )
	{
		Index = __searchForChild( Self->parent, _getKey( Self )[ 0 ] );
		NewKey = substring( _getKey( Self ), PrefixLength, strlen( _getKey( Self ) ), ErrorFlag );
		if( NULL == NewKey )
		{
			return NULL;
		}

		NewParent = makeNewPNode( ErrorFlag );
		if( true == *ErrorFlag )
		{
			free( NewKey );
			return NULL;
		}

		setKey( ( PNode* )NewParent, Key, false, ErrorFlag );
		if( true == *ErrorFlag )
		{
			free( NewKey );
			destroyPNode( NewParent, false );
			return NULL;
		}

		setValue( (PNode* )NewParent, Value, ErrorFlag );
		if( true == *ErrorFlag )
		{
			free( NewKey );
			destroyPNode( NewParent, false );
			return NULL;
		}
		
		setKey( NewParent, Key, false, ErrorFlag );
		if( true == *ErrorFlag )
		{
			free( NewKey );
			destroyPNode( NewParent, false );
			return NULL;
		}

		setKey( Self, NewKey, true, ErrorFlag );
		
		Tmp = Self->parent;
		
		Return = _appendChild( 
				NewParent, 
				Self, 
				false,
				ErrorFlag
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
		CommonKey = substring( CommonKey, 0, PrefixLength, ErrorFlag );
		if( NULL == CommonKey )
		{
			return NULL;
		}
	
		NewKey = substring( Key, PrefixLength, strlen( Key ), ErrorFlag );
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
				ErrorFlag
		);

		free( CommonKey );
		
		if( NULL == NewParent )
		{
			free( NewKey );
			return NULL;
		}

		Index = _insertPosition( NewParent, NewKey[ 0 ] );
		Index = -( Index + 1 );
		Return = _insertChild( NewParent, NewKey, Value, Index, ErrorFlag ); 
		free( NewKey );

		return Return;
	}
}

const PNode* insert(
	PNode* Self,
	const char* Key,
	const char* Value,
	bool Force,
	bool* ErrorFlag
)
{
	short Index;
	Index = _insertPosition( Self, Key[ 0 ] );
	if( -1 < Index )
	{
		return _insert( Self->children[ Index ], Key, Value, Force, ErrorFlag );
	}
	else
	{
		Index = -( Index + 1 );
		return _insertChild( (PNode* )Self, Key, Value, Index, ErrorFlag );
	}
}
/*----------------------------------Debug------------------------------------*/
void printKeys( const PNode* Self, bool* ErrorFlag )
{
	unsigned short Index;
	char* Key;
	
	if( NULL != Self->value )
	{
		Key = getKey( Self, ErrorFlag );
		if( true == *ErrorFlag )
		{
			return;
		}

		printf( "%s:%s\n", Key, Self->value );
		free( Key );
	}
	
	for( Index = 0; Self->sizeOfChildren>Index; Index++ )
	{
		printKeys( Self->children[ Index ], ErrorFlag );
	}
}
/*===================================Utils===================================*/
 char* makeStrCopy( const char* ToCopy, bool* ErrorFlag )
{
	size_t Len;
	char* Dolly;
	
	Len = strlen( ToCopy );
	Len++;//nullbit
	
	Dolly = (char* ) calloc( sizeof( char ), Len );
	if( NULL == Dolly )
	{
		*ErrorFlag = true;
		return NULL;
	}
	
	strcpy( Dolly, ToCopy );
	return Dolly;
}


char* makeEmptyString( bool* ErrorFlag ) 
{
	char* EmptyString;
	
	EmptyString = (char *) malloc( sizeof(char) );
	if( NULL == EmptyString )
	{
		*ErrorFlag = true;
		return NULL;
	}
	
	EmptyString[ 0 ] = '\0';
	return EmptyString;
}
/**
 * Prints a error-message to stderr and quits the programm
 * @param Message | const char* | the message
 */
void errorAndOut( const char* Message )
{
    fprintf( stderr, "%s\n", Message);
    exit( 23 );
}
/**
 * Returns a substring of given String
 * @param Source | char* | the input string
 * @param From | int | the startposition of the subset
 * @param Length | int | Length of the subset
 * @return | char* | the computed subset
 */
char* substring( char* Source, size_t From, size_t Length, bool* ErrorFlag )
{
    char* Return;
    size_t Index1, Index2;
	size_t SourceLength;

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
       return makeStrCopy( Source, ErrorFlag );
    }

    Return = (char *) malloc( ( Length + 1 )*sizeof(char) );
    if( NULL == Return )
    {
    	*ErrorFlag = true;
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
/*===================================Flow=====================================*/
/*=====================================Globals=================================*/
/* make sure we devide between:
 * 0 Wenn die Eingaben formal gültig sind und jedes Wort im Text übersetzt werden kann.
 * 1 Wenn die Eingaben formal gültig sind, im Text aber Wörter auftreten, die nicht übersetzt werden können.
*/
short Return;
PNode* Dictionary;

size_t min( size_t A, size_t B )
{
	return A > B ? A : B;
}

/*----------------------------------DICT--------------------------------------*/
void readInputFile( char* Path );
int nextChar( FILE* Source );
void parseDict( FILE* Source );
bool buildDict( const StringBuffer* Key, const StringBuffer* Value );
void evilFromStdin();
bool pushToBuffer( StringBuffer* Buffer, char InputChar );

int main( int ArgC, char* Arguments[] ) 
{
	bool Error;
	Return = 0;

	if ( 1 == ArgC ) 
	{
		errorAndOut( "Too few arguments provided." );
	}

	if( 2 < ArgC )
	{
		errorAndOut( "To many arguments provided." );
	}

	Error = false;
	
	Dictionary = makeNewPNode( &Error );
	Dictionary->root = true;
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

	destroyPNode( Dictionary, true );
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
		destroyPNode( Dictionary, true );
		errorAndOut( ErrorMsg );
    }

	parseDict( FilePointer );

	fclose( FilePointer );
}

int nextChar( FILE* Source )
{
	return fgetc( Source );
}

void parseDict( FILE* Source )
{
	bool Mode, Pos;
	bool Error;
	int CurrentChar;
	int LookAHead;
	StringBuffer Key;
	StringBuffer Value;
	
	Mode = 0;
	Pos = 0;

	Error = false;

	Key.string = makeEmptyString( &Error );
	if( true == Error )
	{
		fclose( Source );
		destroyPNode( Dictionary, true );
		errorAndOut( "Something went wrong with the memory, jim." );
	}

	Value.string = makeEmptyString( &Error );
	if( true == Error )
	{
		fclose( Source );
		free( Key.string );
		destroyPNode( Dictionary, true );
		errorAndOut( "I cannot get enough mem...." );
	}

	Key.length = 0;
	Value.length = 0;

	LookAHead = nextChar( Source );

	while( EOF != LookAHead )
	{
		CurrentChar = LookAHead;
		LookAHead = nextChar( Source );
		if( ferror( Source ) )
		{
			fclose( Source );
			free( Key.string );
			free( Value.string );
			destroyPNode( Dictionary, true );
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
				EOF == LookAHead
			||
				0 == Pos
			)
			{
				free( Key.string );
				free( Value.string );
				fclose( Source );
				destroyPNode( Dictionary, true );
				errorAndOut( "The given dictionary is incorrectly formed." );
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
				fclose( Source );
				free( Key.string );
				free( Value.string );
				destroyPNode( Dictionary, true );
				errorAndOut( "The given dictionary is incorrectly formed." );
			}

			Trace = Key.string;
			Error = buildDict( &Key, &Value );
			
			free( Key.string );
			free( Value.string );

			if( true == Error )
			{
				destroyPNode( Dictionary, true );
				errorAndOut( "Something exploded downside in the memory lane..." );
			}

			Pos = 0;
			Mode = 0;

			Key.string = makeEmptyString( &Error );
			if( true == Error )
			{
				fclose( Source );
				destroyPNode( Dictionary, true );
				errorAndOut( "There is something wrong with, the memory, Sir!" );
			}
			
			Value.string = makeEmptyString( &Error );
			if( true == Error )
			{
				fclose( Source );
				free( Key.string );
				destroyPNode( Dictionary, true );
				errorAndOut( "There is no space left." );
			}
			
			Key.length = 0;
			Value.length = 0;
			continue;
		}

		/* Valid Chars
		 * Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf.
		 */
		if( 'a' > CurrentChar || 'z' < CurrentChar )
		{
			fclose( Source );
			free( Key.string );
			free( Value.string );
			destroyPNode( Dictionary, true );
			errorAndOut( "The given dictionary is incorrectly formed." );
		}

		Pos = 1;

		/*
		 * a bit slow, could be improved by using buffers + I do not like realloc
		 */
		if( 0 == Mode )
		{
			if( false == pushToBuffer( &Key, CurrentChar ) )
			{
				 fclose( Source );
				 free( Key.string );
				 free( Value.string );
				 destroyPNode( Dictionary, true );
				 errorAndOut( "Memory breach...." );
			}	
		}
		else
		{
			if( false == pushToBuffer( &Value, CurrentChar ) )
			{
				fclose( Source );
				free( Key.string );
				free( Value.string );
				destroyPNode( Dictionary, true );
				errorAndOut( "Just annother memory error message." );
			}
		}
	}

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	free( Key.string );
	free( Value.string );

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	if( 0 != Pos )
	{
		fclose( Source );
		destroyPNode( Dictionary, true );
		errorAndOut( "The given dictionary is incorrectly formed - Missing linefeed before end of file." );
	}
}

bool buildDict( const StringBuffer* Key, const StringBuffer* Value )
{
	bool Error = false;
	if( NULL == insert( 
		Dictionary,
		Key->string,
		Value->string,
		false,
		&Error
	) )
	{
		return false;
	}

#ifdef DEBUGPR
	#ifdef VERBOSE
	bool Interal;
	Interal = false;
	printKeys( Dictionary, &Interal );
	#endif
#endif
	
	return Error;
}

bool pushToBuffer( StringBuffer* Buffer, char InputChar )
{
	size_t Size, Index;
	char* Tmp;

	Size = Buffer->length+2;//new char + \0
	Tmp = (char*) calloc( sizeof(char), Size );
	if( NULL == Tmp )
	{
		return false;
	}
	
	for( Index = 0; Size-1 > Index; Index++ )
	{
		Tmp[ Index ] = Buffer->string[ Index ];
	}
	
	Tmp[ Buffer->length ] = '\0';
	free( Buffer->string );
	Buffer->string = Tmp;
	Buffer->string[ Buffer->length ] = InputChar;
	Buffer->length++;
	Buffer->string[ Buffer->length ] = '\0';
	return true;
}

void evilFromStdin()
{
	StringBuffer Input;
	char* Translation;
	char CurrentChar;
	bool DoneFirstChar;
	bool UpperCase;
	bool Error;

	Error = false;
	
	Input.string = makeEmptyString( &Error );
	if( false == Error )
	{
		destroyPNode( Dictionary, true );
		errorAndOut( "Memory fail...." );
	}
	Input.length = 0;

	DoneFirstChar = false;
	UpperCase = false;
	CurrentChar = nextChar( stdin );
	
	while( EOF != CurrentChar )
	{
		if( ferror( stdin ) )
		{
			// free( Input.string );
			destroyPNode( Dictionary, true );
			errorAndOut( "I/O error when reading from stdin." );
		}

		/* Der eingegebene Text (stdin) sei genau dann gültig, wenn er ausschließlich die Zeichen 10
		 * (line feed) sowie 32–126(inklusive) enthält.
		 */
		if( 10 != CurrentChar || ( 32 > CurrentChar && 126 < CurrentChar ) )
		{
			free( Input.string );
			destroyPNode( Dictionary, true );
			errorAndOut( "Illeagl input from stdin in detected." );
		}

		/* Der erste Buchstabe eines übersetzten
		 * Wortes sei genau dann ein Großbuchstabe, wenn der erste Buchstabe im Ursprungstext groß
		 * ist; alle übrigen Buchstaben des übersetzten Wortes sind immer klein.
		 */
		if( 'A' <= CurrentChar && 'Z' >= CurrentChar )
		{
			if( false == DoneFirstChar )
			{
				UpperCase = true;
				if( false == pushToBuffer( &Input, CurrentChar + TO_LOWER ) )
				{
					free( Input.string );
					destroyPNode( Dictionary, true );
					errorAndOut( "Ohhhhh memory fail......" );
				}

				CurrentChar = nextChar( stdin );
				continue;				
			}
		}
		
		if( ( 'a' > CurrentChar || 'z' < CurrentChar ) && ( 'A' > CurrentChar || 'Z' < CurrentChar ) )
		{
			if( true == DoneFirstChar )
			{
				Translation = findValueByKey( 
					Dictionary,
					Input.string,
					&Error,
					false,
					true
				);

				if( true == Error )
				{
					free( Input.string );
					destroyPNode( Dictionary, true );
					errorAndOut( "Why you do this...there is nothing left..." );
				}

				if( NULL == Translation )
				{
					if( true == UpperCase )
					{
						Input.string[ 0 ] -= TO_LOWER;
					}

					printf( "<%s>", Input.string );
				}
				else
				{
					if( true == UpperCase )
					{
						Translation[ 0 ] -= TO_LOWER;
					}

					printf( "%s", Translation );
					free( Translation );
				}

				free( Input.string );
				Input.string = makeEmptyString( &Error );
				if( true == Error )
				{
					destroyPNode( Dictionary, true );
					errorAndOut( "Nope, there is no memory left...I am sorry" );
				}
			}

			printf( "%c", CurrentChar );
			UpperCase = false;
			DoneFirstChar = false;
			continue;
		}
		
		if( false == pushToBuffer( &Input, CurrentChar ) )
		{   
			free( Input.string );
			destroyPNode( Dictionary, true );
			errorAndOut( "Ohhhhh my the last possible memory fail message - you did it!" );
		}

		DoneFirstChar = true;
		CurrentChar = nextChar( stdin );
	}
}
