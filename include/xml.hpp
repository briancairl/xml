/// @file	xml.hpp
///	@brief 
///	@{
///			XML::reader is hierarchal XML search automoton compatible with all 
///			std::istream and std::istream derived stream types.
///
///			XML::writer is hierarchal XML generation automoton compatible with all 
///			std::istream and std::ostream derived stream types.
///
///			XML::sentry is a read-write capable automoton
///	@}
#define		XML_VER	1
#ifndef		XML_HPP
#define		XML_HPP	XML_VER

#include	<stdint.h>
#include	<stdlib.h>
#include 	<iostream>
#include 	<fstream>
#include 	<sstream>
#include 	<string>
#include 	<mutex>

namespace	XML
{

	#ifndef XML_EXCEPTION
	#define	XML_EXCEPTION	1
	#endif

	#ifndef XML_THREADSAFE	
	#define XML_THREADSAFE	1
	#endif

	#ifndef XML_OP
	#define XML_OP			'<'
	#endif

	#ifndef XML_BS			
	#define XML_BS			'/'
	#endif

	#ifndef XML_CL
	#define XML_CL			'>'
	#endif

	#ifndef XML_EQ
	#define XML_EQ			'='
	#endif

	#ifndef XML_QT
	#define XML_QT			'"'
	#endif

	#ifndef XML_SP
	#define XML_SP			' '
	#endif

	#ifndef XML_TAB
	#define XML_TAB			'\t'
	#endif

	#ifndef XML_NL
	#define XML_NL			'\n'
	#endif


	typedef enum	
	{
		xEOF,
		xGOOD,
		xBAD
	} xmlstat;


	class range
	{
	private:
		bool					inf;
		size_t 					beg;
		size_t					end;
	public:
		range( bool _inf = false ) 
			: beg(0), end(0), inf(_inf)														{}
		~range()																			{}
		inline void				set_beg( const uint32_t& p0 )								{ beg = p0; }
		inline void				set_end( const uint32_t& p1 )								{ end = p1; inf=false; }
		inline const size_t&	get_beg( )													{ return beg; }
		inline const size_t&	get_end( )													{ return end; }
		inline void				set_inf()													{ inf = true; }
		inline void				set_from_beg( std::istream& stream )						{ stream.seekg(std::streamoff(beg)); }
		inline void				set_from_end( std::istream& stream )						{ stream.seekg(std::streamoff(end)); }
		inline bool				is_zero_width()												{ return (!inf)&&(beg==end); }
		inline bool				is_nonzero_width()											{ return ( inf)||(beg< end); }
		inline bool				is_valid_width()											{ return ( inf)||(beg<=end); }
		inline bool				is_within( std::istream& stream )							{ return ( inf)||(((size_t)stream.tellg()>=beg)&&((size_t)stream.tellg()<=end)); }
		inline bool				operator>( const size_t& ulen )								{ return (size_t)(end-beg) < ulen; }
	};



	class reader
	{
	private:
		
		typedef enum
		{
			xBadTag,
			xBadDoc,
			xOpenBegTag,
			xOpenEndTag,
			xCloseTag,
			xCloseInline,
			xChildTag,
			xOK,
			xDONE,
			xFAIL
		} xmlret;

		#if XML_THREADSAFE
		std::mutex 				protex;
		#endif

		bool 					stand_alone;
		std::istream*			stream_ptr;

		size_t					depth;
		std::string				tag;
		xmlstat					stat;

		range					section;
		range					limits;
		range					contents;
		range					param;

		bool					match_tolken( std::string& tolken, range& keep_within );		
		bool					capture_retval( std::ostream& os );
		
		xmlret					find_lock();
		xmlret					find_open_tag();
		xmlret 					find_close_tag();
		xmlret					find_pair();

		void					parse(const char* tag);
	public:

		reader( std::istream& stream );
		reader( const char* fname );

		void					set_stream( std::istream& stream );
		void					set_stream( const char* fname );

		reader&					into(const char* tag);
		reader&					next(const char* tag);

		inline void				rewind()													{ stream_ptr->seekg(0UL,stream_ptr->beg); stat=xGOOD; }
		inline bool				has_content()												{ return contents.is_nonzero_width(); }
		inline bool				has_params()												{ return param.is_nonzero_width();  }
		inline xmlstat			get_stat()													{ return stat; }
		inline size_t			get_depth()													{ return depth; }

		reader&					get_content( std::ostream& os, bool clean = false);	
		reader&					get_content( std::string& str, bool clean = false);	
		reader&					get_section( std::ostream& os );
		reader&					get_section( std::string& str );
		
		template<typename retTy>
		reader&					get_param( std::string tag, retTy& retVal );

		virtual ~reader();
	};




	class writer
	{
	private:
		#if XML_THREADSAFE
		std::mutex 				protex;
		#endif

		std::ostream*			stream_ptr;
		size_t					depth;
		size_t					param_count;
		bool					stand_alone;

		std::ostream&			set_depth();

	public:
		
		typedef enum _xml
		{
			SECTION,
			INLINE
		} xmltype;

		writer( std::ostream& target );
		writer( const char* fname );

		void					set_stream( std::ostream& stream );
		void					set_stream( const char* fname );
		
		std::ostream& 			operator()(size_t i=0UL);
		writer&					beg( const std::string& tag, size_t nparams=0UL, xmltype type=SECTION );
		writer&					end( const std::string& tag );

		template<typename valType>
		writer&  				set_param( const std::string& tag, const valType& val, xmltype type=SECTION );
		
		virtual ~writer();
	};



	class xmlsentry : public reader, writer
	{
	public:
		xmlsentry( std::iostream& stream ) :
			reader(stream), 
			writer(stream)
		{}
	};



	namespace tools
	{
		size_t	collect( std::istream& is, std::ostream& os, const char* tag, bool clean = false );
	}


	#include "xml_templates.hpp"
};
#endif
