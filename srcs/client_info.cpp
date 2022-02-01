#include "client_info.hpp"

void	client_info::fd_in(request_handler& header)	{
	t_func	func[2] = { &client_info::recv_handler,
						&client_info::chunked_handler };

	//std::cout << " Epollin detected!" << std::endl;
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

//	std::cout << " Epollout detected!" << std::endl;
	if (mode > CHUNKED && mode < NONE)
		return (((*this).*func[mode - 2])(header));
}

void	client_info::recv_handler(request_handler& header)	{
	(void)header;
	int		recv_bytes;
	char	buf[MAX_LEN];

	bzero(&buf, MAX_LEN);
	recv_bytes = recv(com_socket, &buf, MAX_LEN, MSG_NOSIGNAL);
//	std::cout << "Data received ! Printing : " << std::endl;
//	std::cout << buf << std::endl;
	if (recv_bytes == -1)
	{
//		std::cout << "RECV ERROR" << std::endl;
		return ;
	}
	else if (recv_bytes == 0)
	{
		mode = COMPUTE;
//		std::cout << "RECV EOF" << std::endl;
	}
 	else if (recv_bytes < MAX_LEN)
	{
		mode = COMPUTE;
//		std::cout << "RECV MSG END" << std::endl;
	}
	rqst.append(buf, recv_bytes);
}

void	client_info::read_handler(request_handler& header)	{
	(void)header;
	int	read_bytes;
	char	buf[MAX_LEN];

	bzero(&buf, MAX_LEN);
//	std::cout << "reading stuff from file" << std::endl;
	read_bytes = read(loc_fd[0], &buf, MAX_LEN);
	if (read_bytes == -1)
	{
//		std::cout << "READ ERROR" << std::endl;
		if (loc_fd[0] != -1)
		{
//			std::cout << "Closing loc_fd[0]" << std::endl;
			close(loc_fd[0]);
			loc_fd[0] = -1;
		}
		mode = SEND;
		return ;
	}
	else if (read_bytes == 0)
	{
		mode = SEND;
//		std::cout << "READ EOF" << std::endl;
		if (loc_fd[0] != -1)
		{
//			std::cout << "Closing loc_fd[0]" << std::endl;
			close(loc_fd[0]);
			loc_fd[0] = -1;
		}
	}
 	else if (read_bytes < MAX_LEN)
	{
		mode = SEND;
//		std::cout << "READ MSG END" << std::endl;
		if (loc_fd[0] != -1)
		{
//			std::cout << "Closing loc_fd[0]" << std::endl;
			close(loc_fd[0]);
			loc_fd[0] = -1;
		}
	}
//	std::cout << "appending resp" << std::endl;
//	std::cout << "resp before = " << std::endl;
//	std::cout << resp << std::endl;
	resp.append(buf, read_bytes);
//	std::cout << "resp after = " << std::endl;
//	std::cout << resp << std::endl;
}

void	client_info::write_handler(request_handler& header)	{
	(void)header;
	int		ret;
	int	wrote_bytes;
	std::string	tmp;

//	std::cout << "Buffer to write is : " << std::endl;
//	std::cout << resp << std::endl;
 	if (resp.length() > (unsigned int)MAX_LEN)
	{
//		std::cout << "FILE TOO BIG" << std::endl;
//		std::cout << "(len is " << resp.length() << ")" << std::endl;
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
//		std::cout << "FILE RECUT" << std::endl;
	}
	wrote_bytes = write(loc_fd[1], resp.c_str(), resp.length());
	if (wrote_bytes == -1)
	{
//		std::cout << "WRITE ERROR" << std::endl;
		//si le write echoue, on reecrit le message dans le buffer, ce dernier sera set
		resp.append(tmp);
		return ;
	}
	else if (wrote_bytes == 0)
	{
//		std::cout << "WRITE EOF" << std::endl;
		tmp.clear();
		if (loc_fd[1] != -1)
		{
//			std::cout << "Closing loc_fd[1]" << std::endl;
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		//ici rqst est deja set, COMPUTE va traiter de nouveu la requete transformee
		ret = header.create_write_resp(loc_path);
		rqst.clear();
		resp = header.get_response();
		if (ret != NONE)
		{
//			std::cout << "Preparing to add file to write response" << std::endl;
			mode = ret;
			header.fill_redir_fd(&loc_fd, &cgi_pid);
		}
		else
			mode = SEND;
		return ;
	}
	else if (wrote_bytes < MAX_LEN)
	{
//		std::cout << "WRITE MSG END" << std::endl;
		tmp.clear();
		if (loc_fd[1] != -1)
		{
//			std::cout << "Closing loc_fd[1]" << std::endl;
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		resp.clear();
		ret = header.create_write_resp(loc_path);
		rqst.clear();
		resp = header.get_response();
	//	std::cout << "Response extracted from header handler: " << std::endl;
	//	std::cout << resp << std::endl;
		if (ret != NONE)
		{
			mode = ret;
//			std::cout << "Copying redir_fd" << std::endl;
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
//	std::cout << "Sending stuff..." << std::endl;
//	std::cout << "Stuff to send is : " << std::endl;
//	std::cout << resp << std::endl;
	if (resp.empty())
	{
		mode = RECV;
		remove();
		return ;
	}
 	else if (resp.length() > (unsigned int)MAX_LEN)
	{
//		std::cout << "MSG TOO LONG" << std::endl;
//		std::cout << "(len is " << resp.length() << ")" << std::endl;
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
//		std::cout << "(len kept is " << tmp.length() << ")" << std::endl;
//		std::cout << "(len sent is " << resp.length() << ")" << std::endl;
//		std::cout << "MSG RECUT" << std::endl;
	}
	sent_bytes = send(com_socket, resp.c_str(), resp.length(), MSG_NOSIGNAL);
//	std::cout << "bytes sent : " << sent_bytes << std::endl;
	if (sent_bytes == -1)
	{
		resp.clear();
		tmp.clear();
//		std::cout << "SEND ERROR" << std::endl;
		remove();
	}
	else if (sent_bytes == 0)
	{
//		std::cout << "SEND EOF" << std::endl;
		resp.clear();
		tmp.clear();
		mode = RECV;
		remove();
	}
	else if (sent_bytes < MAX_LEN)
	{
//		std::cout << "MSG PARTIALLY SENT" << std::endl;
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
//		std::cout << "Empty cgi body buffer, exiting" << std::endl;
		if (loc_fd[1] != -1)
		{
//			std::cout << "Closing loc_fd[1]" << std::endl;
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		mode = CGI_OUT;
		return ;
	}
 	else if (resp.length() > (unsigned int)MAX_LEN)
	{
//		std::cout << "CGI BODY TOO BIG" << std::endl;
//		std::cout << "(len is " << resp.length() << ")" << std::endl;
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
//		std::cout << "CGI BODY RECUT" << std::endl;
	}
//	std::cout << RED "Pipe status in cgi_write:" RESET << std::endl;
//	std::cout << "loc_fd[0] : " << fcntl(loc_fd[0], F_GETFD) << std::endl;
//	std::cout << "loc_fd[1] : " << fcntl(loc_fd[1], F_GETFD) << std::endl;
//	std::cout << "Trying to write on CGI input" << std::endl;
//	std::cout << "Trying to write " << resp.length() << " bytes on fd " << loc_fd[1] << std::endl;

	wrote_bytes = write(loc_fd[1], resp.c_str(), resp.length());

//	std::cout << "Done !" << std::endl;

//	std::cout << wrote_bytes << " bytes written !" << std::endl;
	if (wrote_bytes == -1)
	{
//		std::cout << "CGI WRITE ERROR" << std::endl;
		//si le write echoue, on reecrit le message dans le buffer
//		tmp.append(resp);
		resp.append(tmp);
		tmp.clear();
		return ;

	}
	else if (wrote_bytes == 0)
	{
//		std::cout << "CGI WRITE EOF" << std::endl;
		resp.clear();
		tmp.clear();
		if (loc_fd[1] != -1)
		{
//			std::cout << "Closing loc_fd[1]" << std::endl;
			close(loc_fd[1]);
			loc_fd[1] = -1;
		}
		//ici rqst est deja set, COMPUTE va traiter de nouveu la requete transformee
		mode = CGI_OUT;
	}
	else if (wrote_bytes < MAX_LEN)
	{
//		std::cout << "CGI WRITE MSG END" << std::endl;
		resp = resp.substr(wrote_bytes);
		resp.append(tmp);
	}
	else
	{
		resp = tmp;
	}
//	std::cout << "reseting buffer" << std::endl;
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
//				std::cout << "Cgi buffer has been erased" << std::endl;
			}
		}
			
		resp.append(buf, read_bytes);
//		std::cout << "A CGI READ HAPPENED" << std::endl;
		if (read_bytes == 0)
		{
//			std::cout << "CGI EOF" << std::endl;
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
//				std::cout << "Closing loc_fd[0]" << std::endl;
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
//	std::cout << "Data received ! Printing : " << std::endl;
//	std::cout << buf << std::endl;
	if (recv_bytes == -1)
	{
		if (is_chunked_rqst_fulfilled())
		{
//			std::cout << "CHUNK RECV ERROR" << std::endl;
			rqst.append(chunk_buffer);
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			mode = COMPUTE;
		}
//		std::cout << "RECV ERROR" << std::endl;
		return ;
	}
	else if (recv_bytes == 0)
	{
		if (is_chunked_rqst_fulfilled())
		{
//			std::cout << "CHUNK EOF" << std::endl;
			rqst.append(chunk_buffer);
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			mode = COMPUTE;
		}
//		std::cout << "RECV EOF" << std::endl;
	}
 	else if (recv_bytes < MAX_LEN)
	{
		if (is_chunked_rqst_fulfilled())
		{
//			std::cout << "CHUNK MSG RECV" << std::endl;
			rqst.append(chunk_buffer);
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			mode = COMPUTE;
		}
//		std::cout << "RECV MSG END" << std::endl;
	}
}

void	client_info::compute(request_handler& header)	{
	int	ret;

	ret = is_request_fulfilled();
	if (ret == true)
	{
//		std::cout << BLUE "request : \n" RESET << rqst << std::endl;
		ret = header.reader(rqst);
		if (!ret)
			ret = header.choose_method();
		rqst.clear();
		resp = header.get_response();
		if (ret != NONE)
		{
//			std::cout << "Stuff is happening !" << std::endl;
			mode = ret;
			header.fill_redir_fd(&loc_fd, &cgi_pid);
//			std::cout << RED "Pipe status in compute:" RESET << std::endl;
//			if (loc_fd[0] != -1)
//				std::cout << "loc_fd[0] : " << fcntl(loc_fd[0], F_GETFD) << std::endl;
//			if (loc_fd[1] != -1)
//				std::cout << "loc_fd[1] : " << fcntl(loc_fd[1], F_GETFD) << std::endl;
			if (loc_fd[0] == -1 && loc_fd[1] == -1)
			{
//				std::cout << "There is something wrong with these fds" << std::endl;
				if (loc_fd[0] != -1)
				{
//					std::cout << "Closing loc_fd[0]" << std::endl;
					close(loc_fd[0]);
					loc_fd[0] = -1;
				}
				if (loc_fd[1] != -1)
				{
//					std::cout << "Closing loc_fd[1]" << std::endl;
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
				resp = header.get_body();//on sauvegard le body de la requete precedente pour l'ecrire dans le fichier
			}
			else if (ret == CGI_IN)
			{
//				std::cout << "Body extracted in client buffer" << std::endl;
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
	//SHOULD SEND A BAD REQUEST RESP
	//OR FIGURE WHETHER THE MSG IS CHUNKED
}

bool client_info::is_request_fulfilled()
{
	if (rqst.empty())
		return (false);
	if (chunk_mode == TRANSMISSION_OVER || chunk_mode == BAD_REQUEST)
	{
//		std::cout << "Told you, it's done !" << std::endl;
		chunk_mode = NO_CHUNK;
		return (true);
	}
	if (rqst.find("\r\n\r\n") == std::string::npos)
		return false ;
	if (rq_mode == NORMAL)
	{
//		std::cout << "Checking normal request fullfilment" << std::endl;
		if (rqst.length() >= 4 && rqst.substr(0, 4) != "POST" && rqst.substr(0, 3) != "PUT")
			return (true);
		else if (get_rq_type())
			return (true);
	}
	else
	{
//		std::cout << "Multipart or content-length header detected" << std::endl;
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

//	std::cout << YELLOW "DANS GET_RQ_TYPE\n" RESET << std::endl;
	_cLen = 0;
	if ((pos = rqst.find("Transfer-Encoding: chunked")) != std::string::npos)
	{
//		std::cout << "Chunk header detected at pos: " << pos << std::endl;
		pos = rqst.find("\r\n\r\n");
		chunk_buffer = rqst.substr(pos + 4);
		mode = CHUNKED;
		rqst = rqst.substr(0, pos + 4);
		if (chunk_buffer.empty())
		{
//			std::cout << "Chunk header complete, checking for body" << std::endl;
//			if (rqst.find("Expect: 100-Continue") != std::string::npos)
//			{
//				std::cout << "Sending 100 confirmation header" << std::endl;
//				resp = "HTTP/1.1 100 Continue";
//				chunk_mode = TRANSMIT_100;
//			}
//			else
//			{
//				std::cout << "Enabling network mode to fetch more data" << std::endl;
				chunk_mode = CHUNK_INCOMPLETE;
//			}
		}
		else
		{
//			std::cout << "Chunk body remaining, enabling parsing mode: " << std::endl;
			chunk_mode = CHUNK_PARSING_REQ;
		}
	}
	else if ((pos = rqst.find("Content-Length: ")) != std::string::npos)
	{
		tmp = rqst.substr(pos + strlen("Content-Length: "));
		if ((pos = tmp.find("\r\n")) != std::string::npos)
			tmp = tmp.substr(0, pos);
		ss.str(tmp);
		ss >> _cLen;
//		std::cout << "Len was set:" << _cLen << std::endl;
		if (rqst.substr(rqst.find("\r\n\r\n") + 4).length() >= _cLen)
		{
			rq_mode = NORMAL;
			return (true);
		}
		rq_mode = CLEN;
	}
	else if ((boundary_pos = rqst.find("boundary=")) != string::npos && (boundary_pos += 9 < rqst.length())) // +9 == "boundary=".length, moins deux des premiers '-' +2
	{
		post_boundary = "--" + rqst.substr(boundary_pos, rqst.find_first_of("\r\n", boundary_pos) - boundary_pos);
//		std::cout << "A boundary was found : " << post_boundary << std::endl;
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
//		std::cout << "closing loc fd : " << loc_fd[0] << std::endl;
		close(loc_fd[0]);
		loc_fd[0] = -1;
	}
	if (loc_fd[1] != -1)
	{
//		std::cout << "closing loc fd : " << loc_fd[1] << std::endl;
		close(loc_fd[1]);
		loc_fd[1] = -1;
	}
	if (com_socket != -1)
	{
		std::cout << "Client removed from tracked fd !" << std::endl;
#ifdef _debug_
//		std::cout << "closing socket fd : " << com_socket << std::endl;
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
