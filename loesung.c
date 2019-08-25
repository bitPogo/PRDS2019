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

/*=====================================Types===================================*/
/* This is supposed to live in a seperate file */
typedef struct StringBuffer
{
	char* string;
	unsigned long length;
} StringBuffer;
/*=====================================Globals=================================*/
/* make sure we devide between:
 * 0 Wenn die Eingaben formal gültig sind und jedes Wort im Text übersetzt werden kann.
 * 1 Wenn die Eingaben formal gültig sind, im Text aber Wörter auftreten, die nicht übersetzt werden können.
 */
int Return;

unsigned long min( unsigned long A, unsigned long B ) 
{ 
	return A > B ? A : B; 
}

const char* EmptyString = "\0";

void errorAndOut( const char* Message );
char* substring( char* Source, size_t From, size_t Length, bool* ErrorFlag );
bool startsWith( const char* Str1, const char* Str2 );
bool endsWith( const char* Str1, const char* Str2 );
/*====================================PatricaTrie Defs========================*/
typedef struct PNode {
	struct PNode* parent;
	struct PNode** children;
	unsigned short sizeOfChildren;
	char* value;
	char* key;
} PNode;

const char* getKey( const PNode* Self, bool* ErrorFlag );
size_t longestPrefix( const PNode* Self, const char* Key );
const PNode* _findByKey( const PNode* Self, const char* Key, bool MatchExact );

PNode* makeNewPNode( bool* ErrorFlag )
{
	PNode* NewNode;
	NewNode = (PNode* ) malloc( sizeof(PNode* ) );
	if( NULL == NewNode )
	{
		*ErrorFlag = true;
		return NULL;
	}

	NewNode->children = NULL;
	NewNode->sizeOfChildren = 0;
	NewNode->key = NULL;
	NewNode->value = NULL;

	return NewNode;
}

// TODO Stack req
void destroyPNode( PNode* Node, bool Recursive )
{
	long Index;

	if( true == Recursive )
	{
		for( Index = 0; Node->sizeOfChildren>Index; Index++ )
		{
			destroyPNode( Node->children[ Index ], Recursive );
		}
	}

	if( NULL != Node->children )
	{
		free( Node->children );
	}

	if( NULL != Node->key )
	{
		free( Node->key );
	}

	if( NULL != Node->value )
	{
		free( Node->value );
	}

	free( Node );
}

/*------------------------------------Basement-------------------------------*/
const char* _getKey( const PNode* Self )
{
	if( NULL == Self->key )
	{
		return EmptyString;
	}
	else
	{
		return Self->key;
	}
}

const char* _getPrefix( const PNode* Self, bool* ErrorFlag )
{
	if( NULL == Self->parent )
	{
		return EmptyString;	
	}
	else
	{
		return getKey( Self->parent, ErrorFlag );
	}
}

size_t longestPrefix( const PNode* Self, const char* Key )
{
	size_t To, Index;

	To = min( strlen( Key ), strlen( _getKey( Self ) ) );
	for( Index = 0; To > Index; Index++ )
	{
		if( Key[ Index ] != Self->key[ Index ] )
		{
			return Index;
		}
	}
	return To;
}

char* makeStrCopy( const char* ToCopy, bool* ErrorFlag )
{
	size_t Len;
	char* Dolly;

	Len = strlen( ToCopy );
	Len++;//nullbit
	Dolly = (char* ) calloc( Len, sizeof( char ) );
	if( NULL == Dolly )
	{
		*ErrorFlag = true;
		return NULL;
	}

	strcpy( Dolly, ToCopy );
	return Dolly;
}

void setValue( PNode* Self, const char* Value, bool* ErrorFlag )
{
	if( NULL != Value )
	{
		Self->value = makeStrCopy( Value, ErrorFlag );
	}
}

void setKey( PNode* Self, const char* Key, bool* ErrorFlag )
{
	Self->key = makeStrCopy( Key, ErrorFlag );
}
/*----------------------------------Reading----------------------------------*/
const char* getKey( const PNode* Self, bool* ErrorFlag )
{
	const char* Tmp;
	char* Return;
	unsigned long PayloadSize;

	if( NULL == Self->parent )
	{
		return EmptyString;
	}
	else
	{
		Tmp = _getPrefix( Self, ErrorFlag );
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
		
		free( (char *)Tmp );
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

long __searchForChild( const PNode* Self, char Key )
{
	size_t Start, End, Middle;

	if( 0 == Self->sizeOfChildren )
	{
		return -1;
	}

	if( _getKey( Self->children[ 0 ] )[0] > Key )
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
	long Index;
	const char* MyKey;
	char* StrStart;
	const PNode* CurrentNode;

	CurrentNode = Self;
	StrStart = (char* )Key;

	/* Should use _commonPrefix, but does not regarding Stack requorements */
	while( true )
	{
		if( 0 == CurrentNode->sizeOfChildren )
		{
			return NULL;
		}
		else
		{
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
				CurrentNode = CurrentNode->children[ Index ];
			}
			else
			{
				return NULL;
			}
		}
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

long _insertPosition( const PNode* Self, char Key )
{
	size_t Start, End, Middle;

	if( 0 == Self->sizeOfChildren )
	{
		return -1;
	}
	
	if( _getKey( Self->children[ 0 ] )[0] > Key )
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

	return -( Start + 1 );	
}

PNode* __appendChild( PNode* Where, PNode* NewChild, long Index, bool* ErrorFlag )
{
	PNode** NewChildren;
	if( 0 == Where->sizeOfChildren )
	{   
		NewChildren = (PNode** ) calloc( 1, sizeof( PNode* ) );
	}
	else
	{   
		NewChildren = (PNode** ) realloc( Where->children, ( ( Where->sizeOfChildren+1 )*sizeof( PNode* ) ) );// this might a bit slow, but better for the memory
	}
	
	if( NULL == NewChildren )
	{
		*ErrorFlag = true;
		return NULL;
	}

	NewChild->parent = (PNode* ) Where;
	NewChildren[ Index ] = NewChild;
	Where->sizeOfChildren++;
	Where->children = NewChildren;
	return NewChild;
}

PNode* _insertChild(
	PNode* Where,
	const char* Key,
	const char* Value,
	long Index,	
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

	setKey( NewChild, Key, ErrorFlag );
	if( true == *ErrorFlag )
	{
		*ErrorFlag = true;
		free( NewChild );
		return NULL;
	}

	setValue( NewChild, Value, ErrorFlag );
	if( true == *ErrorFlag )
	{
		*ErrorFlag = true;
		free( NewChild->key );
		free( NewChild );
		return NULL;
	}

	Return = __appendChild( Where, NewChild, Index, ErrorFlag );
	if( NULL == Return )
	{
		free( NewChild->value );
		free( NewChild->key );
		free( NewChild );
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
	long Index;

	Index = __searchForChild( Self, Child->key[ 0 ] );

	if( 0 <= Index )
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
		return __appendChild(
			Self,
			Child,
			Index,
			ErrorFlag
		);
	}
}

const PNode* _insert( 
	PNode* Self, 
	const char* OrgKey,
	const char* Value,
	bool Force,
	bool* ErrorFlag 
)
{
	size_t PrefixLength, KeyLength, InsertKeyLength;
	long Index;
	PNode* NewParent;
	PNode* Return;
	PNode* Tmp;
	char* Key;
	char* NewKey;
	char* NewKey2;
	char* CommonKey;
	
	Key = (char*) OrgKey;
	PrefixLength = longestPrefix( Self, Key );
	KeyLength = strlen( _getKey( Self ) );
	InsertKeyLength = strlen( Key );

	if( 0 == PrefixLength )
	{
        if( 0 == Self->sizeOfChildren )
		{
			return _insertChild( Self, Key, Value, -1, ErrorFlag );
		}

		Index = _insertPosition( Self, Key[ 0 ] );
		
		if( 0 <= Index )
		{
			return _insert( Self->children[ Index ], Key, Value, Force, ErrorFlag );
		}
		else
		{
			Index = -( Index + 1 ); 
			return ( const PNode* )_insertChild( (PNode* )Self, Key, Value, Index, ErrorFlag );
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
		}
            
		return Self;
	}
	else if( PrefixLength == KeyLength )
	{
		Key = (char *)Key + ( PrefixLength * sizeof( char ) );
		if( 0 == Self->sizeOfChildren )
		{
			return _insertChild( Self, Key, Value, -1, ErrorFlag );
		}

		Index = _insertPosition( Self, Key[ 0 ] );
		
		if( 0 <= Index )
		{
			return _insert( Self->children[ Index ], Key, Value, Force, ErrorFlag );
		}
		else
		{
			Index = -( Index + 1 );
			return ( const PNode* )_insertChild( (PNode* )Self, Key, Value, Index, ErrorFlag );
		}
	}
	else if( PrefixLength == InsertKeyLength )
	{
		NewKey = (char *)_getKey( Self );
		NewKey = substring( NewKey, PrefixLength, strlen( NewKey ), ErrorFlag );
		if( NULL == NewKey )
		{
			return NULL;
		}

		NewKey2 = (char *)_getKey( Self );
		NewKey2 = substring( NewKey2, 0, PrefixLength, ErrorFlag );
		if( NULL == NewKey2 )
		{
			free( NewKey2 );
			return NULL;
		}

		NewParent = makeNewPNode( ErrorFlag );
		if( true == *ErrorFlag )
		{
			free( NewKey );
			free( NewKey2 );
			return NULL;
		}

		setValue( (PNode* )NewParent, Value, ErrorFlag );
		if( true == *ErrorFlag )
		{
			free( NewKey );
			free( NewKey2 );
			free( NewParent );
			return NULL;
		}
		
		NewParent->key = NewKey2;
		free( Self->key );
		Self->key = NewKey;
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
		
		Return = _appendChild( 
			Tmp, 
			NewParent,
			true,
			ErrorFlag 
		);

		if( NULL == Return )
		{
			destroyPNode( NewParent, false );
			return NULL;
		}

		return NewParent;
	}
	else
	{
		CommonKey = (char *) _getKey( Self );
		CommonKey = substring( Self->key, 0, PrefixLength, ErrorFlag );
		NewParent = (PNode* )_insert( 
				Self->parent,
				CommonKey,
				NULL,
				true,
				ErrorFlag
		);
		
		if( NULL == NewParent )
		{
			free( CommonKey );
			return NULL;
		}

		return _insert( NewParent, Key, Value, false, ErrorFlag ); 
	}
}
/*----------------------------------Debug------------------------------------*/
void printKeys( const PNode* Self, bool* ErrorFlag )
{
	unsigned short Index;
	const char* Key;
	if( NULL != Self->value )
	{
		Key = getKey( Self, ErrorFlag );
		if( &Key != &EmptyString ) 
		{
			printf( "%s:%s\n", getKey( Self, ErrorFlag ), Self->value );
		}

		if( true == *ErrorFlag )
		{
			return;
		}

        for( Index = 0; Self->sizeOfChildren>Index; Index++ )
		{
			printKeys( Self->children[ Index ], ErrorFlag );
		}
	}
}
/*===================================Utils===================================*/
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

    if( NULL == Source )
    {
        return NULL;
    }

    if( 0 == From && strlen( Source ) == Length)
    {

        Return = (char *) malloc( ( strlen( Source )+1 )*sizeof(char) );
        if( NULL == Return )
        {
        	*ErrorFlag = true;
			return NULL;
		}

        strcpy( Return , Source );
        return Return;
    }

    Return = (char *) malloc( ( Length + 1 )*sizeof(char) );
    if( NULL == Return )
    {
    	*ErrorFlag = true;
		return NULL;
	}

    for( Index1=From, Index2=0; Length>Index2; Index1++, Index2++)
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
/*----------------------------------DICT--------------------------------------*/
void readInputFile( char* Path );
int nextDict( FILE* Source );
void parseDict( FILE* Source );
bool buildDict( const StringBuffer* Key, const StringBuffer* Value );

int main( int ArgC, char* Arguments[] ) 
{
	Return = 0;
	
	if ( 1 == ArgC ) 
	{
		errorAndOut( "Too few arguments provided." );
	}

	if( 2 < ArgC )
	{
		errorAndOut( "To many arguments provided." );
	}

#ifdef DEBUGPR
	printf( "Start preproc" );
#endif
	readInputFile( Arguments[ 1 ] );
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

	parseDict( FilePointer );

	fclose( FilePointer );
}

int nextDict( FILE* Source )
{
	return fgetc( Source );
}

void parseDict( FILE* Source )
{
	unsigned long Pos;
	bool Mode;
	int CurrentChar;
	int LookAHead;
	StringBuffer Key;
	StringBuffer Value;
	char* Tmp;
	
	Mode = 0;
	Pos = 0;

	Key.string = (char*) calloc( sizeof(char), 1 );
	if( NULL == Key.string )
	{
		fclose( Source );
		errorAndOut( "Something went wrong with the memory, jim." );
	}

	Value.string = (char*) calloc( sizeof(char), 1 );
	if( NULL == Key.string )
	{
		fclose( Source );
		free( Key.string );
		errorAndOut( "I cannot get enough mem...." );
	}

	Key.string[ 0 ] = '\0';
	Value.string[ 0 ] = '\0';
	Key.length = 0;
	Value.length = 0;

	LookAHead = nextDict( Source );

	while( EOF != LookAHead )
	{
		CurrentChar = LookAHead;
		LookAHead = nextDict( Source );
		if( ferror( Source ) )
		{
			fclose( Source );
			free( Key.string );
			free( Value.string );
			errorAndOut( "I/O error when reading." );
		}

		Pos++;
		
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
				1 == Pos
			)
			{
				free( Key.string );
				free( Value.string );
				fclose( Source );
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
				errorAndOut( "The given dictionary is incorrectly formed." );
			}

			buildDict( &Key, &Value );

			free( Key.string );
			free( Value.string );

			Pos = 0;
			Mode = 0;

			Key.string = (char*) calloc( sizeof(char), 1 );
			if( NULL == Key.string )
			{
				fclose( Source );
				errorAndOut( "There is something wrong with, the memory, Sir!" );
			}
			
			Value.string = (char*) calloc( sizeof(char), 1 );
			if( NULL == Key.string )
			{
				fclose( Source );
				free( Key.string );
				errorAndOut( "There is no space left." );
			}
			
			Key.string[ 0 ] = '\0';
			Value.string[ 0 ] = '\0';
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
			errorAndOut( "The given dictionary is incorrectly formed." );
		}

		/*
		 * a bit slow, could be improved by using buffers
		 */
		if( 0 == Mode )
		{
			Tmp = (char*) realloc( Key.string, sizeof( char )*( Key.length + 2 ) ); // Null byte + new Char
			if( NULL == Tmp )
			{
				fclose( Source );
				free( Key.string );
				free( Value.string );
				errorAndOut( "Memory breach...." );
			}

			Key.string = Tmp;
			Key.string[ Key.length ] = (char) CurrentChar;
			Key.length++;
			Key.string[ Key.length ] = '\0';
		}
		else
		{
			Tmp = (char*) realloc( Value.string, sizeof( char )*( Value.length + 2 ) );
			if( NULL == Tmp )
			{
				fclose( Source );
				free( Key.string );
				free( Value.string );
				errorAndOut( "Just annother memory error message." );
			}

			Value.string = Tmp;
			Value.string[ Value.length ] = (char) CurrentChar;
			Value.length++;
			Value.string[ Value.length ] = '\0';
		}
	}

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	free( Key.string );
	free( Value.string );

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	if( 0 != Pos )
	{
		fclose( Source );
		errorAndOut( "The given dictionary is incorrectly formed - Missing linefeed before end of file." );
	}
}

bool buildDict( const StringBuffer* Key, const StringBuffer* Value )
{
	printf( "%s:%s\n", Key->string, Value->string );
	return true;
}
