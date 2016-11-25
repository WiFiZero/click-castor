/*
 * csclient.cc
 * Douglas S. J. De Couto
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include <algorithm>
#include <iostream>

#include "csclient.hh"


#define check_init() do { if (!_init) return init_err; } while (false);

using namespace std;

ControlSocketClient::err_t
ControlSocketClient::configure(unsigned int host_ip, unsigned short port)
{
  if (_init)
    return reinit_err;

  _host = host_ip;
  _port = port;

  _fd = socket(PF_INET, SOCK_STREAM, 0);
  if (_fd < 0)
    return sys_err;

  /*
   * connect to remote ControlSocket
   */
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = _host;
  sa.sin_port = htons(port);

  char namebuf[32];
  snprintf(namebuf, 32, "%u.%u.%u.%u:%hu",
	   (_host & 0xff) >> 0,
	   (_host & 0xff00) >> 8,
	   (_host & 0xff0000) >> 16,
	   (_host & 0xff000000) >> 24,
	   port);
  _name = namebuf;

  int res = connect(_fd, (struct sockaddr *)  &sa, sizeof(sa));
  if (res < 0) {
    int save_errno = errno;
    ::close(_fd);
    errno = save_errno;
    return sys_err;
  }

  int major, minor;
  size_t slash, dot;

  /*
   * check that we get the expected banner
   */
  string buf;
  err_t err = readline(buf);
  if (err != no_err) {
    int save_errno = errno;
    ::close(_fd);
    errno = save_errno;
    return err;
  }

  slash = buf.find('/');
  dot = (slash != string::npos ? buf.find('.', slash + 1) : string::npos);
  if (slash == string::npos || dot == string::npos) {
    ::close(_fd);
    return click_err; /* bad format */
  }

  /*
   * check ControlSocket protocol version
   */
  major = atoi(buf.substr(slash + 1, dot - slash - 1).c_str());
  minor = atoi(buf.substr(dot + 1, buf.size() - dot - 1).c_str());
  if (major != PROTOCOL_MAJOR_VERSION ||
      minor < PROTOCOL_MINOR_VERSION) {
    ::close(_fd);
    return click_err; /* wrong version */
  }

  _init = true;
  return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::close()
{
  check_init();
  _init = false;
  int res = ::close(_fd);
  if (res < 1)
    return sys_err;
  else
    return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::readline(string &buf)
{
  assert(_fd);

#define MAX_LINE_SZ 1024 /* arbitrary... to prevent weirdness */

  /*
   * keep calling read() to get one character at a time, until we get
   * a line.  not very ``efficient'', but who cares?
   */
  char c = 0;
  buf.resize(0);
  do {
    int res = ::read(_fd, (void *) &c, 1);
    if (res < 0)
      return sys_err;
    if (res != 1)
      return sys_err;
    buf += c;
    if (buf.size() > MAX_LINE_SZ)
      return click_err;
  }
  while (c != '\n');

  return no_err;
}


int
ControlSocketClient::get_resp_code(string line)
{
  if (line.size() < 3)
    return -1;
  return atoi(line.substr(0, 3).c_str());
}


int
ControlSocketClient::get_data_len(string line)
{
  unsigned int i;
  for (i = 0; i < line.size() && !isdigit((unsigned char) line[i]); i++)
    ; // scan string
  if (i == line.size())
    return -1;
  return atoi(line.substr(i, line.size() - i).c_str());
}


ControlSocketClient::err_t
ControlSocketClient::read(string el, string handler, string &response)
{
  check_init();

  if (el.size() > 0)
    handler = el + "." + handler;
  string cmd = "READ " + handler + "\n";

  int res = ::write(_fd, cmd.c_str(), cmd.size());
  if (res < 0)
    return sys_err;
  if ((size_t) res != cmd.size())
    return sys_err;

  string cmd_resp;
  string line;
  do {
    err_t err = readline(line);
    if (err != no_err)
      return err;
    if (line.size() < 4)
      return click_err;
    cmd_resp += line;
  }
  while (line[3] == '-');

  int code = get_resp_code(line);
  if (code != CODE_OK && code != CODE_OK_WARN)
    return handle_err_code(code);

  res = readline(line);
  if (res < 0)
    return click_err;
  int num = get_data_len(line);
  if (num < 0)
    return click_err;

  response.resize(0);
  if (num == 0)
    return no_err;

  char *buf = new char[num];
  int num_read = 0;
  while (num_read < num) {
    res = ::read(_fd, buf + num_read, num - num_read);
    if (res < 0) {
      delete[] buf;
      return sys_err;
    }
    num_read += res;
  }

  response.append(buf, num);
  delete[] buf;

  return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::read(string el, string handler, char *buf, int &bufsz)
{
  string resp;
  err_t err = read(el, handler, resp);
  if (err != no_err)
    return err;

  bufsz = min((size_t) bufsz, resp.size());

  memcpy(buf, resp.c_str(), bufsz);
  if (resp.size() > (size_t) bufsz)
    return too_short;
  else
    return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::write(string el, string handler, const char *buf, int bufsz)
{
  check_init();

  if (el.size() > 0)
    handler = el + "." + handler;
  char cbuf[10];
  snprintf(cbuf, sizeof(cbuf), "%d", bufsz);
  string cmd = "WRITEDATA " + handler + " " + cbuf + "\n";

  int res = ::write(_fd, cmd.c_str(), cmd.size());
  if (res < 0)
    return sys_err;
  if ((size_t) res != cmd.size())
    return sys_err;

  res = ::write(_fd, buf, bufsz);
  if (res < 0)
    return sys_err;
  if (res != bufsz)
    return sys_err;

  string cmd_resp;
  string line;
  do {
    err_t err = readline(line);
    if (err != no_err)
      return err;
    if (line.size() < 4)
      return click_err;
    cmd_resp += line;
  }
  while (line[3] == '-');

  int code = get_resp_code(line);
  if (code != CODE_OK && code != CODE_OK_WARN)
    {
      cout << "CCCC " << code << endl;
    return handle_err_code(code);
    }

  return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::write(string el, string handler, string data)
{
  return write(el, handler, data.c_str(), data.size());
}


ControlSocketClient::err_t
ControlSocketClient::handle_err_code(int code)
{
  switch (code) {
  case CODE_SYNTAX_ERR: return click_err; break;
  case CODE_UNIMPLEMENTED: return click_err; break;
  case CODE_NO_ELEMENT: return no_element; break;
  case CODE_NO_HANDLER: return no_handler; break;
  case CODE_HANDLER_ERR: return handler_err; break;
  case CODE_PERMISSION: return handler_no_perm; break;
  default: return click_err; break;
  }
  return click_err;
}



vector<string>
ControlSocketClient::split(string s, size_t offset, char terminator)
{
  vector<string> v;
  size_t pos = offset;
  size_t len = s.size();
  while (pos < len) {
    size_t start = pos;
    while (pos < len && s[pos] != terminator)
      pos++;
    if (start < pos || pos < len)
      v.push_back(s.substr(start, pos - start));
    pos++;
  }
  return v;
}


ControlSocketClient::err_t
ControlSocketClient::get_config_el_names(vector<string> &els)
{
  string resp;
  err_t err = read("", "list", resp);
  if (err != no_err)
    return err;

  /* parse how many els */
  int i = resp.find('\n');
  int num = atoi(resp.substr(0, i).c_str());


  els = split(resp, i + 1, '\n');
  if (els.size() != (size_t) num)
    return handler_bad_format;

  return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::get_string_vec(string el, string h, vector<string> &v)
{
  string resp;
  err_t err = read(el, h, resp);
  if (err != no_err)
    return err;

  v = split(resp, 0, '\n');
  return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::get_el_handlers(string el, vector<handler_info_t> &handlers)
{
  vector<handler_info_t> v;
  vector<string> vh;

  string buf;
  err_t err = read(el, "handlers", buf);
  if (err != no_err)
    return err;

  vh = split(buf, 0, '\n');
  for (vector<string>::iterator i = vh.begin(); i != vh.end(); i++) {
    string &s = *i;
    size_t j;
    for (j = 0; j < s.size() && !isspace((unsigned char) s[j]); j++)
      ; /* find record split -- don't use s.find because could be any whitespace */
    if (j == s.size())
      return click_err;
    handler_info_t hi;
    hi.element_name = el;
    hi.handler_name = trim(s.substr(0, j));
    while (j < s.size() && isspace((unsigned char) s[j]))
      j++;
    for ( ; j < s.size(); j++) {
	if (tolower((unsigned char) s[j]) == 'r')
	    hi.can_read = true;
	else if (tolower((unsigned char) s[j]) == 'w')
	    hi.can_write = true;
	else if (isspace((unsigned char) s[j]))
	    break;
    }
    v.push_back(hi);
  }

  handlers = v;
  return no_err;
}


ControlSocketClient::err_t
ControlSocketClient::check_handler(string el, string h, bool is_write, bool &exists)
{
  check_init();

  if (el.size() > 0)
    h = el + "." + h;
  string cmd = (is_write ? "CHECKWRITE " : "CHECKREAD ") + h + "\n";

  int res = ::write(_fd, cmd.c_str(), cmd.size());
  if (res < 0)
    return sys_err;
  if ((size_t) res != cmd.size())
    return sys_err;

  string cmd_resp;
  string line;
  do {
    err_t err = readline(line);
    if (err != no_err)
      return err;
    if (line.size() < 4)
      return click_err;
    cmd_resp += line;
  }
  while (line[3] == '-');

  int code = get_resp_code(line);
  switch (code) {
  case CODE_OK:
  case CODE_OK_WARN:
    exists = true;
    return no_err;;
  case CODE_NO_ELEMENT:
  case CODE_NO_HANDLER:
  case CODE_HANDLER_ERR:
  case CODE_PERMISSION:
    exists = false;
    return no_err;
  case CODE_UNIMPLEMENTED:
    if (el.size() == 0)
      return handle_err_code(code); /* no workaround for top-level router handlers */
    else
      return check_handler_workaround(el, h, is_write, exists);
  default:
    return handle_err_code(code);
  }
}



ControlSocketClient::err_t
ControlSocketClient::check_handler_workaround(string el, string h, bool is_write, bool &exists)
{
  /*
   * If talking to an old ControlSocket, try the "handlers" handler
   * instead.
   */

  vector<handler_info_t> v;
  err_t err = get_el_handlers(el, v);
  if (err != no_err)
    return err;

  for (vector<handler_info_t>::iterator i = v.begin(); i != v.end(); i++) {
    if (i->handler_name == h) {
      if ((is_write && i->can_write) ||
	  (!is_write && i->can_read))
	exists = true;
      else
	exists = false;
      return no_err;
    }
  }

  exists = false;
  return no_err;
}


string
ControlSocketClient::trim(string s)
{
  size_t start, end;
  for (start = 0; start < s.size() && isspace((unsigned char) s[start]); start++)
    ; /* */
  for (end = s.size(); end > 0 && isspace((unsigned char) s[end - 1]); end--)
    ; /* */

  if (start >= end)
    return "";

  return s.substr(start, end - start);
}
