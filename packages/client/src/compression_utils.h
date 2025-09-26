#pragma once

// std
#include <vector>
#include <stdexcept>

// zlib
#include "zlib.h"

[[nodiscard]] inline std::vector<unsigned char> gunzipInMemory(const unsigned char* src, size_t len)
{
  z_stream strm{};
  strm.next_in  = const_cast<Bytef*>(src);
  strm.avail_in = static_cast<uInt>(len);

  if (inflateInit2(&strm, 15 + 32) != Z_OK)
    throw std::runtime_error("inflateInit2 failed");

  std::vector<unsigned char> out(64 * 1024);
  int ret;
  do {
    if (strm.total_out >= out.size())
      out.resize(out.size() * 2);

    strm.next_out  = out.data() + strm.total_out;
    strm.avail_out = static_cast<uInt>(out.size() - strm.total_out);

    ret = inflate(&strm, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
      inflateEnd(&strm);
      throw std::runtime_error("inflate failed");
    }
  } while (ret != Z_STREAM_END);

  out.resize(strm.total_out);
  inflateEnd(&strm);
  return out;
}