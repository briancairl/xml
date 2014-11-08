/// @file	XML.cpp
///	@brief 
///	@{
///			reader is hierarchal XML search automoton compatible with the WebTools::Cache::cache
///	@}
#include	"xml.hpp"


namespace XML
{
	/// @brief	[in-class]	TRUE if associated stream is healthy
	#define XML_SEARCH_VALID			(!this->stream_ptr->fail())

	/// @brief 	[in-class]	Position within current associated stream
	#define XML_POS						(static_cast<size_t>(this->stream_ptr->tellg()))
	
	/// @brief 	[in-class]	Associated stream
	#define XML_STREAM					(*this->stream_ptr)
	
	/// @brief 	[in-class]	Enables thread scoped guarding macro
	#if 	XML_THREADSAFE
	#define XML_THREADGAURD				std::lock_guard<std::mutex>	__gaurd(protex);
	#else
	#define XML_THREADGAURD
	#endif


	///	@brief	Existing-stream constructor
	///			Associates an existing input stream with the sentry
	///
	/// @param	stream	input stream 
	reader::reader( std::istream& stream ) :
		tag(tag), 
		limits(true), 
		contents(true), 
		param(true),
		section(true),
		depth(0UL),
		stat(xGOOD),
		stand_alone(false)
	{
		/// Set the input stream
		set_stream(stream);

		if(!XML_SEARCH_VALID)
			stat = xBAD;
	}



	///	@brief	Stand-alone constructor
	///			Associates an input stream with the sentry
	///
	/// @param	fname	input file name
	reader::reader( const char* fname ) :
		tag(tag), 
		limits(true), 
		contents(true), 
		param(true),
		section(true),
		depth(0UL),
		stat(xGOOD),
		stand_alone(true)
	{
		/// Set the input stream from file
		stream_ptr = new std::ifstream(fname); 

		if(!XML_SEARCH_VALID)
			stat = xBAD;
	}


	/// @brief 	Associates a new stream with the sentry 
	/// @param	stream	input stream
	void reader::set_stream( std::istream& stream ) 
	{XML_THREADGAURD

		if(stand_alone)
			delete stream_ptr;
		
		stream_ptr 	= &stream;
		stand_alone = false;
	}


	/// @brief 	Associates a new file with the sentry 
	/// @param	fname	input file name
	void reader::set_stream( const char* fname )
	{XML_THREADGAURD

		if(stand_alone)
			delete stream_ptr;
		
		stream_ptr 	= new std::ifstream(fname); 
		stand_alone = true;
	}



	reader& reader::into(const char* tag)
	{
		if(contents.is_nonzero_width())
		{
			limits = contents;
			limits.set_from_beg(XML_STREAM);			
		}
		return next(tag);
	}


	reader&  reader::next(const char* tag)
	{XML_THREADGAURD
		parse(tag);
		return (*this);
	}


	/// @brief	Sets active tag and finds next associated pair
	///			[private]
	void	reader::parse(const char* tag)
	{
		/// Set new tag
		this->tag = tag;

		/// Check tag validity
		if( this->tag.empty() || (find_pair()!=xOK) )
			stat = xBAD;
	}


	/// @brief	Checks if next stream sequence is the specified tag
	///			[private]
	bool reader::match_tolken( std::string& tolken, range& keep_within )
	{ 
		char atc;
		for( size_t idx=0 ; idx<tolken.size(); idx++ )
		{
			if(XML_SEARCH_VALID)
			{
				stream_ptr->get(atc);
				if( (atc==' ') )
					idx--;
				else
				if( (atc == XML_CL) || (atc != tolken[idx]) )
					return false;
			}
			else
			{
				stat = xEOF;
				return false;
			}
		}
		return true;
	}



	bool reader::capture_retval( std::ostream& os )
	{
		char	atc;
		size_t	qc = 0UL;
		while( XML_SEARCH_VALID && param.is_within(XML_STREAM) )
		{
			stream_ptr->get(atc);
			switch(atc)
			{
			case XML_QT:
				if(++qc==2UL)
					return true;
				break;
			case ' ':
				if(qc!=1UL)
					break;
			default:
				if(qc==1UL)
					os<<atc;
				break;
			}
		}
		stat = xEOF;
		return false;
	}


	reader::xmlret reader::find_lock()
	{
		char atc;
		while(XML_SEARCH_VALID)
		{
			stream_ptr->get(atc);
			switch(atc)
			{
				case XML_BS:
				if(stream_ptr->peek()==XML_CL)
				{
					stream_ptr->get(atc);
					return xCloseInline;
				}
				else
				{
					stat = xBAD;
					return xBadDoc;
				}
				case XML_OP:
				if(stream_ptr->peek()==XML_BS)
				{
					stream_ptr->get(atc);
					return xOpenEndTag;
				}
				else
					return xOpenBegTag;
				case XML_CL:
					return xCloseTag;
				default:
					continue;
			}
		}
		stat = xEOF;
		return xBadDoc;
	}





	/// @brief	
	reader::xmlret reader::find_open_tag()
	{
		while(XML_SEARCH_VALID)
		{
			switch(find_lock())
			{
			case xOpenBegTag: // "<..."
				++depth;
				section.set_beg(XML_POS-1UL);
				if(match_tolken(tag,limits))
				{
					param.set_beg(static_cast<size_t>(stream_ptr->tellg()));
					switch(find_lock())
					{		
						case xCloseTag:
							param	.set_end(XML_POS-1UL);
							contents.set_beg(XML_POS+1UL);
							return xOK;
						case xCloseInline:
							param	.set_end(XML_POS-1UL);

							contents.set_beg(XML_POS+1UL);
							contents.set_end(XML_POS+1UL);
							return xDONE;
						default:
							stat = xBAD;
							return xFAIL;
					}
				}
				continue;
			case xOpenEndTag: // "</..."
				--depth;
				continue;
			case xBadDoc:
				stat = xBAD;
				return xFAIL;
			default:
				continue;
			}
		}
		stat = xBAD;
		depth = 0UL; // you've derailed
		return xFAIL;
	}




	/// @brief	
	reader::xmlret reader::find_close_tag()
	{
		while(XML_SEARCH_VALID)
		{
			switch(find_lock())
			{
			case xOpenBegTag:	// "<..."
				++depth;
				continue;
			case xOpenEndTag:	// "</..."
				if(match_tolken(tag,limits))
				{
					contents.set_end(XML_POS-tag.size()-3UL);
					if(find_lock()==xCloseTag)
					{
						section.set_end(XML_POS);
						return xOK;
					}
					else
						return xFAIL;
				}
			case xCloseInline:	// ".../>"
				--depth;
				continue;
			case xBadDoc:
				return xFAIL;
			default:
				continue;
			}
		}
		depth = 0UL; // you've derailed
		return xFAIL;
	}



	/// @brief	
	reader::xmlret reader::find_pair()
	{
		switch(find_open_tag())
		{
		case xOK:	// tagged item has closing tag
			return find_close_tag(); 
		case xDONE: // tagged item was inline
			return xOK;
		default:	// document error (malformation)
			return xFAIL;
		}
	}



	/// @brief	
	reader&	reader::get_content( std::ostream& os, bool clean )
	{XML_THREADGAURD
		bool closed(true);
		char atc;
		contents.set_from_beg(XML_STREAM);
		while(XML_SEARCH_VALID && contents.is_within(XML_STREAM))
		{
			stream_ptr->get(atc);
			if(clean)
			{
				switch(atc)
				{
				case XML_OP:
					closed = false;
					continue;
				case XML_CL:
					closed = true;
					continue;
				default:
					break;
				}
			}
			if( closed )
				os << atc;
		}
		return (*this); 
	}
	reader&	reader::get_content( std::string& str, bool clean )
	{XML_THREADGAURD
		bool closed(true);
		char atc;
		contents.set_from_beg(XML_STREAM);
		while(XML_SEARCH_VALID && contents.is_within(XML_STREAM))
		{
			stream_ptr->get(atc);
			if(clean)
			{
				switch(atc)
				{
				case XML_OP:
					closed = false;
					continue;
				case XML_CL:
					closed = true;
					continue;
				default:
					break;
				}
			}
			if( closed )
				str.push_back(atc);
		}
		return (*this); 
	}


	reader&	reader::get_section( std::ostream& os )
	{XML_THREADGAURD
		char atc;
		section.set_from_beg(XML_STREAM);	
		while(XML_SEARCH_VALID && section.is_within(XML_STREAM))
		{
			stream_ptr->get(atc);
			os << atc;
		}
		return (*this); 
	}
	reader&	reader::get_section( std::string& str )
	{XML_THREADGAURD
		char atc;
		section.set_from_beg(XML_STREAM);	
		while(XML_SEARCH_VALID && section.is_within(XML_STREAM))
		{
			stream_ptr->get(atc);
			str.push_back(atc);
		}
		return (*this); 
	}


	reader::~reader() 
	{
		if(stand_alone)
			delete stream_ptr;
	}





	std::ostream&	writer::set_depth()
	{
		for( size_t idx = 0UL; idx < depth; idx++ )
			(*stream_ptr) << XML_TAB;
		return (*stream_ptr);
	}


	writer::writer( std::ostream& stream ) : 
		stream_ptr(&stream), depth(0UL), param_count(0UL), stand_alone(false)
	{}



	writer::writer( const char* fname ) : 
		depth(0UL), param_count(0UL), stand_alone(true)
	{
		stream_ptr = new std::ofstream(fname);
	}
	

	void writer::set_stream( std::ostream& stream ) 
	{XML_THREADGAURD

		if(stand_alone)
			delete stream_ptr;
		
		stream_ptr 	= &stream;
		stand_alone = false;
	}



	void writer::set_stream( const char* fname )
	{XML_THREADGAURD

		if(stand_alone)
			delete stream_ptr;
		
		stream_ptr 	= new std::ofstream(fname); 
		stand_alone = true;
	}



	std::ostream& writer::operator()(size_t i) 
	{ 
		set_depth();
		return (*stream_ptr); 
	}



	writer& writer::beg( const std::string& tag, size_t nparams, xmltype type )
	{
		param_count = nparams;
		if(param_count)
			set_depth() << XML_OP << tag;
		else
		{
			if(type==SECTION)
				set_depth() << XML_OP << tag << XML_CL << XML_NL;
			else
				set_depth() << XML_OP << tag << XML_BS << XML_CL << XML_NL;
		}
		depth++;
		return (*this);
	}


	writer&  writer::end( const std::string& tag )
	{
		depth--;
		set_depth() << XML_OP <<  XML_BS << tag << XML_CL << XML_NL;
		return (*this);
	}


	writer::~writer() 
	{
		if(stand_alone)
			delete stream_ptr;
	}




	namespace tools
	{
		size_t	collect( std::istream& is, std::ostream& os, const char* tag, bool clean )
		{
			bool		good = true;
			size_t		count=0UL;
			reader	sentry(is);

			while(sentry.get_stat()==xGOOD)
				sentry.next(tag).get_content(os,clean);

			return count;
		}
	}
}