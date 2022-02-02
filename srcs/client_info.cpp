#include "client_info.hpp"

void	client_info::fd_in(request_handler& header)	{
	t_func	func[2] = { &client_info::recv_handler,
						&client_info::chunked_handler };

	if (mode < COMPUTE)
		return (((*this).*func[mode])(header));
}

void	client_info::fd_out(request_handler& header)	{
	t_func	func[6] = {	&client_info::compute,
						&client_info::write_handler,
						&client_info::read_handler,
						&client_info::cgi_write_handler,
						&client_info::cgi_resp_handler,
						&client_info::send_handler};

	if (mode > CHUNKED && mode < NONE)
		return (((*this).*func[mode - 2])(header));
}

void	client_info::recv_handler(request_handler& header)	{
	(void)header;
	int		recv_bytes;
	char	buf[MAX_LEN];

	bzero(&buf, MAX_LEN);
	recv_bytes = recv(com_socket, &buf, MAX_LEN, MSG_NOSIGNAL);
#ifdef _debug_
	std::cout << "Data received !";
#endif
#ifdef _debug_full_
	std::cout << "Printing : " << std::endl;
	std::cout << buf << std::endl;
#endif
	if (recv_bytes == -1)
	{
#ifdef _debug_
		std::cout << "RECV ERROR" << std::endl;
#endif
		remove();
		return ;
	}
	else if (recv_bytes == 0)
	{
		mode = COMPUTE;
#ifdef _debug_
		std::cout << "RECV EOF" << std::endl;
#endif
	}
 	else if (recv_bytes < MAX_LEN)
	{
		mode = COMPUTE;
#ifdef _debug_
		std::cout << "RECV MSG END" << std::endl;
#endif
	}
	rqst.append(buf, recv_bytes);
}

void	client_info::read_handler(request_handler& header)	{
	(void)header;
	int	read_bytes;
	char	buf[MAX_LEN];

	bzero(&buf, MAX_LEN);
#ifdef _debug_
	std::cout << "reading stuff from file" << std::endl;
#endif
	read_bytes = read(loc_fd[0], &buf, MAX_LEN);
	if (read_bytes == -1)
	{
#ifdef _debug_
		std::cout << "READ ERROR" << std::endl;
#endif
		if (loc_fd[0] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[0]" << std::endl;
#endif
			close(loc_fd[0]);
			loc_fd[0] = -1;
		}
		mode = SEND;
		return ;
	}
	else if (read_bytes == 0)
	{
		mode = SEND;
#ifdef _debug_
		std::cout << "READ EOF" << std::endl;
#endif
		if (loc_fd[0] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[0]" << std::endl;
#endif
			close(loc_fd[0]);
			loc_fd[0] = -1;
		}
	}
 	else if (read_bytes < MAX_LEN)
	{
		mode = SEND;
#ifdef _debug_
		std::cout << "READ MSG END" << std::endl;
#endif
		if (loc_fd[0] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[0]" << std::endl;
#endif
			close(loc_fd[0]);
			loc_fd[0] = -1;
		}
	}
	resp.append(buf, read_bytes);
}

void	client_info::write_handler(request_handler& header)	{
	(void)header;
	int		ret;
	int	wrote_bytes;
	std::string	tmp;

#ifdef _debug_full_
	std::cout << "Buffer to write is : " << std::endl;
	std::cout << resp << std::endl;
#endif
 	if (resp.length() > (unsigned int)MAX_LEN)
	{
#ifdef _debug_
		std::cout << "FILE TOO BIG" << std::endl;
		std::cout << "(len is " << resp.length() << ")" << std::endl;
#endif
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
#ifdef _debug_
		std::cout << "FILE RECUT" << std::endl;
#endif
	}
	wrote_bytes = write(loc_fd[1], resp.c_str(), resp.length());
	if (wrote_bytes == -1)
	{
#ifdef _debug_
		std::cout << "WRITE ERROR" << std::endl;
#endif
		resp.append(tmp);
		return ;
	}
	else if (wrote_bytes == 0)
	{
#ifdef _debug_
		std::cout << "WRITE EOF" << std::endl;
#endif
		tmp.clear();
		if (loc_fd[1] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[1]" << std::endl;
#endif
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		ret = header.create_write_resp(loc_path);
		rqst.clear();
		resp = header.get_response();
		if (ret != NONE)
		{
#ifdef _debug_
			std::cout << "Preparing to add file to write response" << std::endl;
#endif
			mode = ret;
			header.fill_redir_fd(&loc_fd, &cgi_pid);
		}
		else
			mode = SEND;
		return ;
	}
	else if (wrote_bytes < MAX_LEN)
	{
#ifdef _debug_
		std::cout << "WRITE MSG END" << std::endl;
#endif
		tmp.clear();
		if (loc_fd[1] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[1]" << std::endl;
#endif
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		resp.clear();
		ret = header.create_write_resp(loc_path);
		rqst.clear();
		resp = header.get_response();
#ifdef _debug_full_
		std::cout << "Response extracted from header handler: " << std::endl;
		std::cout << resp << std::endl;
#endif
		if (ret != NONE)
		{
			mode = ret;
#ifdef _debug_
			std::cout << "Copying redir_fd" << std::endl;
#endif
			header.fill_redir_fd(&loc_fd, &cgi_pid);
		}
		else
			mode = SEND;
		return ;
	}
	resp = tmp;
}

void	client_info::send_handler(request_handler& header)	{
	int	sent_bytes;
	std::string	tmp;

	(void)header;
#ifdef _debug_
	std::cout << "Sending stuff..." << std::endl;
#endif
#ifdef _debug_full
	std::cout << "Stuff to send is : " << std::endl;
	std::cout << resp << std::endl;
#endif
	if (resp.empty())
	{
		mode = RECV;
		return ;
	}
 	else if (resp.length() > (unsigned int)MAX_LEN)
	{
#ifdef _debug_
		std::cout << "MSG TOO LONG" << std::endl;
		std::cout << "(len is " << resp.length() << ")" << std::endl;
#endif
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
#ifdef _debug_
		std::cout << "(len kept is " << tmp.length() << ")" << std::endl;
		std::cout << "(len sent is " << resp.length() << ")" << std::endl;
		std::cout << "MSG RECUT" << std::endl;
#endif
	}
	sent_bytes = send(com_socket, resp.c_str(), resp.length(), MSG_NOSIGNAL);
#ifdef _debug_
	std::cout << "bytes sent : " << sent_bytes << std::endl;
#endif
	if (sent_bytes == -1)
	{
		resp.clear();
		tmp.clear();
#ifdef _debug_
		std::cout << "SEND ERROR" << std::endl;
#endif
		remove();
	}
	else if (sent_bytes == 0)
	{
#ifdef _debug_
		std::cout << "SEND EOF" << std::endl;
#endif
		resp.clear();
		tmp.clear();
		mode = RECV;
		remove();
	}
	else if (sent_bytes < MAX_LEN)
	{
#ifdef _debug_
		std::cout << "MSG PARTIALLY SENT" << std::endl;
#endif
		resp = resp.substr(sent_bytes);
		resp.append(tmp);
	}
	else
		resp = tmp;
}

void	client_info::cgi_write_handler(request_handler& header)	{
	(void)header;
	int	wrote_bytes;
	std::string	tmp;

	if (resp.empty())
	{
#ifdef _debug_
		std::cout << "Empty cgi body buffer, exiting" << std::endl;
#endif
		if (loc_fd[1] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[1]" << std::endl;
#endif
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		mode = CGI_OUT;
		return ;
	}
 	else if (resp.length() > (unsigned int)MAX_LEN)
	{
#ifdef _debug_
		std::cout << "CGI BODY TOO BIG" << std::endl;
		std::cout << "(len is " << resp.length() << ")" << std::endl;
#endif
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
#ifdef _debug_
		std::cout << "CGI BODY RECUT" << std::endl;
#endif
	}
#ifdef _debug_
	std::cout << "Trying to write on CGI input" << std::endl;
	std::cout << "Trying to write " << resp.length() << " bytes on fd " << loc_fd[1] << std::endl;
#endif

	wrote_bytes = write(loc_fd[1], resp.c_str(), resp.length());

#ifdef _debug_
	std::cout << "Done !" << std::endl;

	std::cout << wrote_bytes << " bytes written !" << std::endl;
#endif
	if (wrote_bytes == -1)
	{
#ifdef _debug_
		std::cout << "CGI WRITE ERROR" << std::endl;
#endif
		resp.append(tmp);
		tmp.clear();
		return ;

	}
	else if (wrote_bytes == 0)
	{
#ifdef _debug_
		std::cout << "CGI WRITE EOF" << std::endl;
#endif
		resp.clear();
		tmp.clear();
		if (loc_fd[1] != -1)
		{
#ifdef _debug_
			std::cout << "Closing loc_fd[1]" << std::endl;
#endif
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		mode = CGI_OUT;
	}
	else if (wrote_bytes < MAX_LEN)
	{
#ifdef _debug_
		std::cout << "CGI WRITE MSG END" << std::endl;
#endif
		resp = resp.substr(wrote_bytes);
		resp.append(tmp);
	}
	else
	{
		resp = tmp;
	}
#ifdef _debug_
	std::cout << "reseting buffer" << std::endl;
#endif
}

void	client_info::cgi_resp_handler(request_handler& header)	{
	(void)header;
	int	read_bytes;
	char	buf[MAX_LEN];

	bzero(&buf, MAX_LEN);
	read_bytes = read(loc_fd[0], &buf, MAX_LEN);
	if (read_bytes == -1)
	{
		return ;
	}
	else
	{
		if (CGI_MODE == 1)
		{
			struct stat	plop;

			if (lstat("./files/tmp/cgi_buffer",  &plop) != -1)
			{
				std::remove("./files/tmp/cgi_buffer");
#ifdef _debug_
				std::cout << "Cgi buffer has been erased" << std::endl;
#endif
			}
		}
			
		resp.append(buf, read_bytes);
#ifdef _debug_
		std::cout << "A CGI READ HAPPENED" << std::endl;
#endif
		if (read_bytes == 0)
		{
#ifdef _debug_
			std::cout << "CGI EOF" << std::endl;
#endif
			waitpid(cgi_pid, NULL, 0);
			cgi_pid =  -1;
			header.set_body(resp);
			resp.clear();
			header.clean_body();
			header.cgi_writer();
			resp = header.get_response();
			mode = SEND;
			if (loc_fd[0] != -1)
			{
#ifdef _debug_
				std::cout << "Closing loc_fd[0]" << std::endl;
#endif
				close(loc_fd[0]);
				loc_fd[0] = -1;
			}
		}
	}
}

void	client_info::chunked_handler(request_handler& header)	{
	(void)header;
	int		recv_bytes;
	char	buf[MAX_LEN];

	bzero(&buf, MAX_LEN);
	recv_bytes = recv(com_socket, &buf, MAX_LEN, MSG_NOSIGNAL);
	if (recv_bytes != -1)
		chunk_buffer.append(buf, recv_bytes);
	#ifdef _debug_
	std::cout << "Data received !";
	#endif
	#ifdef _debug_full_
	std::cout << "Printing : " << std::endl;
	std::cout << buf << std::endl;
	#endif
	if (recv_bytes == -1)
	{
		if (is_chunked_rqst_fulfilled())
		{
#ifdef _debug_
			std::cout << "CHUNK RECV ERROR" << std::endl;
	#endif
			rqst.append(chunk_buffer);
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			mode = COMPUTE;
		}
#ifdef _debug_
		std::cout << "RECV ERROR" << std::endl;
	#endif
		return ;
	}
	else if (recv_bytes == 0)
	{
		if (is_chunked_rqst_fulfilled())
		{
#ifdef _debug_
			std::cout << "CHUNK EOF" << std::endl;
	#endif
			rqst.append(chunk_buffer);
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			mode = COMPUTE;
		}
#ifdef _debug_
		std::cout << "RECV EOF" << std::endl;
	#endif
	}
 	else if (recv_bytes < MAX_LEN)
	{
		if (is_chunked_rqst_fulfilled())
		{
#ifdef _debug_
			std::cout << "CHUNK MSG RECV" << std::endl;
	#endif
			rqst.append(chunk_buffer);
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			mode = COMPUTE;
		}
#ifdef _debug_
		std::cout << "RECV MSG END" << std::endl;
	#endif
	}
}

void	client_info::compute(request_handler& header)	{
	int	ret;

	ret = is_request_fulfilled();
	if (ret == true)
	{
#ifdef _debug_full_
		std::cout << BLUE "request : \n" RESET << rqst << std::endl;
	#endif
		ret = header.reader(rqst);
		if (!ret)
			ret = header.choose_method();
		rqst.clear();
		resp = header.get_response();
		if (ret != NONE)
		{
#ifdef _debug_
			std::cout << "Changing client state" << std::endl;
	#endif
			mode = ret;
			header.fill_redir_fd(&loc_fd, &cgi_pid);
			if (loc_fd[0] == -1 && loc_fd[1] == -1)
			{
#ifdef _debug_
				std::cout << "Invalid fd in compute" << std::endl;
	#endif
				if (loc_fd[0] != -1)
				{
#ifdef _debug_
					std::cout << "Closing loc_fd[0]" << std::endl;
	#endif
					close(loc_fd[0]);
					loc_fd[0] = -1;
				}
				if (loc_fd[1] != -1)
				{
#ifdef _debug_
					std::cout << "Closing loc_fd[1]" << std::endl;
	#endif
					close(loc_fd[1]);
					loc_fd[1] = -1;
				}
				mode = SEND;
				return ;
			}
			if (ret == WRITE)
			{
				loc_path = header.get_path();
#ifdef _debug_
				std::cout << "Location path is : " << loc_path << std::endl;
#endif
				resp = header.get_body();
			}
			else if (ret == CGI_IN)
			{
#ifdef _debug_
				std::cout << "Body extracted in client buffer" << std::endl;
#endif
				resp = header.get_body();
			}
			return ;
		}
		mode = SEND;
	}
	else if (!(mode == CHUNKED))
	{
		mode = RECV;
	}
}

bool client_info::is_request_fulfilled()
{
	if (rqst.empty())
		return (false);
	if (chunk_mode == TRANSMISSION_OVER || chunk_mode == BAD_REQUEST)
	{
#ifdef _debug_
		std::cout << "Chunk Transmission over!" << std::endl;
#endif
		chunk_mode = NO_CHUNK;
		return (true);
	}
	if (rqst.find("\r\n\r\n") == std::string::npos)
		return false ;
	if (rq_mode == NORMAL)
	{
#ifdef _debug_
		std::cout << "Checking normal request fullfilment" << std::endl;
#endif
		if (rqst.length() >= 4 && rqst.substr(0, 4) != "POST" && rqst.substr(0, 3) != "PUT")
			return (true);
		else if (get_rq_type())
			return (true);
	}
	else
	{
#ifdef _debug_
		std::cout << "Multipart or content-length header detected" << std::endl;
#endif
		if (rq_mode == CLEN && is_clen_rqst_fulfilled())
			return (true);
		else if (rq_mode == MULTIPART && is_multipart_rqst_fulfilled())
			return (true);
	}
	return (false);
}

bool client_info::get_rq_type()
{
	size_t				pos;
	std::string			tmp;
	std::stringstream	ss;
	size_t boundary_pos;

#ifdef _debug_
	std::cout << YELLOW "DANS GET_RQ_TYPE\n" RESET << std::endl;
#endif
	_cLen = 0;
	if ((pos = rqst.find("Transfer-Encoding: chunked")) != std::string::npos)
	{
#ifdef _debug_
		std::cout << "Chunk header detected at pos: " << pos << std::endl;
#endif
		pos = rqst.find("\r\n\r\n");
		chunk_buffer = rqst.substr(pos + 4);
		mode = CHUNKED;
		rqst = rqst.substr(0, pos + 4);
		if (chunk_buffer.empty())
				chunk_mode = CHUNK_INCOMPLETE;
		else
			chunk_mode = CHUNK_PARSING_REQ;
	}
	else if ((pos = rqst.find("Content-Length: ")) != std::string::npos)
	{
		tmp = rqst.substr(pos + strlen("Content-Length: "));
		if ((pos = tmp.find("\r\n")) != std::string::npos)
			tmp = tmp.substr(0, pos);
		ss.str(tmp);
		ss >> _cLen;
#ifdef _debug_
		std::cout << "Len was set:" << _cLen << std::endl;
#endif
		if (rqst.substr(rqst.find("\r\n\r\n") + 4).length() >= _cLen)
		{
			rq_mode = NORMAL;
			return (true);
		}
		rq_mode = CLEN;
	}
	else if ((boundary_pos = rqst.find("boundary=")) != string::npos && (boundary_pos += 9 < rqst.length())) 
	{
		post_boundary = "--" + rqst.substr(boundary_pos, rqst.find_first_of("\r\n", boundary_pos) - boundary_pos);
#ifdef _debug_
		std::cout << "A boundary was found : " << post_boundary << std::endl;
#endif
		rq_mode = MULTIPART;
	}
	else
	{
		rq_mode = NORMAL;
		return (true);
	}
	return (false);
}

bool client_info::is_multipart_rqst_fulfilled()	{
	size_t	pos;

	if (!post_boundary.empty() && (pos = rqst.find(post_boundary + "--")) != std::string::npos)
	{
		rq_mode = NORMAL;
		return (true);
	}
	return (false);
}

bool client_info::is_clen_rqst_fulfilled()	{
	std::string	tmp;

	if (_cLen != 0)
	{
		if (_cLen <= rqst.length())
		if (rqst.substr(rqst.find("\r\n\r\n") + 4).length() >= _cLen)
		{
			rqst = rqst.substr(0, _cLen + rqst.substr(rqst.find("\r\n\r\n") + 4).length());
			rq_mode = NORMAL;
			return (true);
		}
	}
	return (false);
}

bool client_info::is_chunked_rqst_fulfilled()	{
	if (chunk_buffer.empty())
		return (true);
	if (chunk_buffer.find("0\r\n\r\n") != std::string::npos)
		return (true);
	return (false);
}

void	client_info::set_recv_mode()	{
	struct epoll_event	ev;

	bzero(&ev, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = com_socket;
	if (epoll_ctl(_epoll->_epoll_fd, EPOLL_CTL_MOD, com_socket, &ev))
	{
		std::cerr << "Failed to switch to reception mode" << std::endl;
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
}

void	client_info::set_send_mode()	{
	struct epoll_event	ev;

	bzero(&ev, sizeof(ev));
	ev.events = EPOLLOUT;
	ev.data.fd = com_socket;
	if (epoll_ctl(_epoll->_epoll_fd, EPOLL_CTL_MOD, com_socket, &ev))
	{
		std::cerr << "Failed to switch to send mode" << std::endl;
		throw std::runtime_error("ERROR IN EPOLL_CTL MANIPULATION");
	}
}

void	client_info::time_reset()	{
	time(&rqst_time_start);
}

void	client_info::remove()	{
	if (loc_fd[0] != -1)
	{
#ifdef _debug_
		std::cout << "closing loc fd : " << loc_fd[0] << std::endl;
#endif
		close(loc_fd[0]);
		loc_fd[0] = -1;
	}
	if (loc_fd[1] != -1)
	{
#ifdef _debug_
		std::cout << "closing loc fd : " << loc_fd[1] << std::endl;
#endif
		close(loc_fd[1]);
		loc_fd[1] = -1;
	}
	if (com_socket != -1)
	{
		std::cout << "Client removed from tracked fd !" << std::endl;
#ifdef _debug_
		std::cout << "closing socket fd : " << com_socket << std::endl;
#endif
		if (epoll_ctl(_epoll->_epoll_fd, EPOLL_CTL_DEL, com_socket, NULL))
		{
			perror("epoll_ctl");
			throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
		}
		if (close(com_socket))
		{
			perror("close fd");
			throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
		}
		com_socket = -1;
	}
}
