/// template impl


/// @brief  Retrieves parameter value corresponding to a supplied tag.
///
///      Parameters must be retrieved in order, however foward parameters
///      may be skipped.
///
/// @param  tage  parameter name whose value will be retrieved
/// @param  retVal  parameter value
///  @return this
template<typename retTy>
reader& reader::get_param( std::string tag, retTy& retVal )
{
  if(param.is_zero_width())
  {
    return (*this);
  }
  else if(stat==xGOOD)
  {
    std::stringstream conv;
    
    if(!param.is_within(*stream_ptr))
      param.set_from_beg(*stream_ptr);

    while(!stream_ptr->fail() && param.is_within(*stream_ptr))
    {
      if(match_tolken(tag,param) && capture_retval(conv))
      {
        conv >> retVal;
        break;
      }
    }
  }
  return (*this); 
}



template<typename valType>
writer&  writer::set_param( const std::string& tag, const valType& val, xmltype type )
{
  if(param_count--)
  {
    (*stream_ptr) << ' ' << tag << '=' << '"' << val << '"';
  }
  else
  {
    throw exception::BadParamCountFile();
  }

  if(!param_count)
  {
    if(type==SECTION)
      (*stream_ptr) << '>' << '\n';
    else
    {
      depth--;
      (*stream_ptr) << '/' << '>' << '\n';
    }
  }
  return (*this);
}