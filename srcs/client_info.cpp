#include "client_info.hpp"

void	client_info::fd_in(request_handler& header)	{
	t_func	func[3] = { &client_info::recv_handler,
						&client_info::chunked_handler,
						&client_info::compute };

	//handles EPOLLIN EVENTS
	if (mode <= COMPUTE)
		return (((*this).*func[mode])(header));
}

void	client_info::fd_out(request_handler& header)	{
	t_func	func[6] = { &client_info::chunked_handler,
						&client_info::compute,
						&client_info::write_handler,
						&client_info::read_handler,
						&client_info::send_handler,
						&client_info::cgi_resp_handler };

	//handles EPOLLOUT EVENTS
	if (mode > RECV && mode < NONE)
		return (((*this).*func[mode - 1])(header));
}

void	client_info::recv_handler(request_handler& header)	{
	(void)header;
	int		recv_bytes;
	char	buf[MAX_LEN];

	recv_bytes = recv(com_socket, &buf, MAX_LEN, MSG_NOSIGNAL);
	if (recv_bytes == -1)
	{
		std::cout << "RECV ERROR" << std::endl;
		return ;
	}
	else if (recv_bytes == 0)
	{
		mode = COMPUTE;
		std::cout << "RECV EOF" << std::endl;
	}
 	else if (recv_bytes < MAX_LEN)
	{
		mode = COMPUTE;
		std::cout << "RECV MSG END" << std::endl;
	}
	rqst.append(buf, recv_bytes);
}

void	client_info::read_handler(request_handler& header)	{
	(void)header;
	int	read_bytes;
	char	buf[MAX_LEN];

	read_bytes = read(loc_fd, &buf, MAX_LEN);
	if (read_bytes == -1)
	{
		std::cout << "READ ERROR" << std::endl;
		close(loc_fd);
		mode = SEND;
		return ;
	}
	else if (read_bytes == 0)
	{
		mode = SEND;
		std::cout << "READ EOF" << std::endl;
		close(loc_fd);
	}
 	else if (read_bytes < MAX_LEN)
	{
		mode = SEND;
		std::cout << "READ MSG END" << std::endl;
		close(loc_fd);
	}
	else
		std::cout << "A READ HAPPENED" << std::endl;
	resp.append(buf, read_bytes);
}

void	client_info::write_handler(request_handler& header)	{
	(void)header;
	int	wrote_bytes;
	std::string	tmp;

 	if (resp.length() > (unsigned int)MAX_LEN)
	{
		std::cout << "FILE TOO BIG" << std::endl;
		std::cout << "(len is " << resp.length() << ")" << std::endl;
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
		std::cout << "FILE RECUT" << std::endl;
	}
	wrote_bytes = write(loc_fd, resp.c_str(), resp.length());
	if (wrote_bytes == -1)
	{
		std::cout << "WRITE ERROR" << std::endl;
		//si le write echoue, on reecrit le message dans le buffer, ce dernier sera set
//		tmp.append(resp);
		resp.append(tmp);
		return ;
	}
	else if (wrote_bytes == 0)
	{
		std::cout << "WRITE EOF" << std::endl;
		tmp.clear();
		close(loc_fd);
		//ici rqst est deja set, COMPUTE va traiter de nouveu la requete transformee
		mode = COMPUTE;
	}
	else if (wrote_bytes < MAX_LEN)
	{
		std::cout << "WRITE MSG END" << std::endl;
		tmp.clear();
		close(loc_fd);
		mode = COMPUTE;
	}
	resp = tmp;
	time_reset();
}

void	client_info::send_handler(request_handler& header)	{
	int	sent_bytes;
	std::string	tmp;

	(void)header;
 	if (resp.length() > (unsigned int)MAX_LEN)
	{
		std::cout << "MSG TOO LONG" << std::endl;
		std::cout << "(len is " << resp.length() << ")" << std::endl;
		tmp = resp.substr(MAX_LEN);
		resp = resp.substr(0, MAX_LEN);
		std::cout << "MSG RECUT" << std::endl;
	}
	sent_bytes = send(com_socket, resp.c_str(), resp.length(), MSG_NOSIGNAL);
	if (sent_bytes == -1)
	{
		std::cout << "SEND ERROR" << std::endl;
		//tmp.append(resp);
		//resp = tmp;
		resp.append(tmp);
		return ;
	}
	else if (sent_bytes == 0)
	{
		std::cout << "SEND EOF" << std::endl;
		tmp.clear();
		mode = RECV;
		remove();
//		time_reset();
	}
	else if (sent_bytes < MAX_LEN)
	{
		std::cout << "MSG SENT" << std::endl;
		tmp.clear();
		mode = RECV;
		remove();
//		time_reset();
	}
	resp = tmp;
	tmp.clear();
}

void	client_info::cgi_resp_handler(request_handler& header)	{
	(void)header;
	int	read_bytes;
	char	buf[MAX_LEN];

	read_bytes = read(loc_fd, &buf, MAX_LEN);
	if (read_bytes == -1)
		return ;
	else
	{
		resp.append(buf, read_bytes);
		std::cout << "A CGI READ HAPPENED" << std::endl;
		if (read_bytes == 0)
		{
			std::cout << "CGI EOF" << std::endl;
			waitpid(-1, NULL, 0);
			header.set_body(resp);
			resp.clear();
			header.clean_body();
			header.cgi_writer();
			resp = header.get_response();
			mode = SEND;
			close(loc_fd);
		}
	}
}

void	client_info::chunked_handler(request_handler& header)	{
	(void)header;
	int		bytes_exchanged;
	char	buf[MAX_LEN];

	if (chunk_mode == CHUNK_INCOMPLETE)
	{
//		std::cout << "Asking for new chunks" << std::endl;
		bytes_exchanged = recv(com_socket, &buf, MAX_LEN, MSG_DONTWAIT);
		if (bytes_exchanged == -1)
		{
//			std::cout << "Error in chunk reception, trying again" << std::endl;
			return ;
		}
		else
			std::cout << "New chunk part received !" << std::endl;
		chunk_buffer.append(buf, bytes_exchanged);
		chunk_mode = CHUNK_PARSING_REQ;
	}
	else if (chunk_mode == CHUNK_PARSING_REQ)
	{
		is_chunked_rqst_fulfilled();
	}
	else if (chunk_mode == CHUNK_COMPLETE)
	{
		resp = "HTTP/1.1 100 Continue\r\n"; 
		bytes_exchanged = send(com_socket, resp.c_str(), resp.length(), MSG_NOSIGNAL);
		if (chunk_buffer.empty())
		{
			std::cout << "Chunk header complete, checking for body" << std::endl;
			chunk_mode = CHUNK_INCOMPLETE;
		}
		else
		{
			std::cout << "Chunk body remaining, enabling parsing mode" << std::endl;
			chunk_mode = CHUNK_PARSING_REQ;
		}
		resp.clear();
	}
	else if (chunk_mode == CHUNK_COMPLETE)
	{
		resp = "HTTP/1.1 400 Bad Request\r\n"; 
		std::cout << "Bad request" << std::endl;
		bytes_exchanged = send(com_socket, resp.c_str(), resp.length(), MSG_NOSIGNAL);
		if (bytes_exchanged != -1)
		{
			mode = RECV;
			resp.clear();
			chunk_buffer.clear();
		}
	}
	else if (chunk_mode == TRANSMISSION_OVER)
	{
		std::cout << "Chunked transmission complete !" << std::endl;
//		std::cout << "Decoded request:\n" << rqst << std::endl;
		mode = COMPUTE;
	}
}

void	client_info::compute(request_handler& header)	{
	int	ret;

	ret = is_request_fulfilled();
	if (ret == true)
	{
//		std::cout << BLUE "request : \n" RESET << rqst << std::endl;
		header.reader(rqst);
		ret = header.choose_method();
		rqst.clear();
		resp = header.get_response();
		if (ret != NONE)
		{
			mode = ret;
			loc_fd = header.get_redir_fd();
			if (ret == WRITE)
			{
				rqst = header.get_response(); //on sauvegarde des infos de header pour former une nouvelle requete
				resp = header.get_body();//on sauvegard le body de la requete precedente pour l'ecrire dans le fichier
			}
			else if (ret == CGI_OUT)
				resp = header.get_body();
			return ;
		}
		mode = SEND;
	}
	else if (!(mode == CHUNKED))
	{
//		if (rq_mode == NORMAL)
//			remove();
//		else
//			time_reset();
		mode = RECV;
	}
	//SHOULD SEND A BAD REQUEST RESP
	//OR FIGURE WHETHER THE MSG IS CHUNKED
}

bool client_info::is_request_fulfilled()
{
	// cout << BLUE "DANS IS_REQUEST_FULFILLED, size of current reqst : " << clients[client_fd].rqst.length() <<"\n" RESET;

	// for (size_t i = clients[client_fd].rqst.length() - 4; i < clients[client_fd].rqst.length(); ++i)
	// 	cout << "clients[client_fd][i] :[" << clients[client_fd].rqst[i] << "]\n";
	if (chunk_mode == TRANSMISSION_OVER)
	{
		chunk_mode = NO_CHUNK;
		return (true);
	}
	if (rqst.find("\r\n\r\n") == std::string::npos)
		return false ;
	if (rq_mode == NORMAL)
	{
		if (get_rq_type())
			return (true);
	}
	else
	{
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

	cout << YELLOW "DANS IS_POST_REQUEST_FULFILLED\n" RESET << std::endl;
	_cLen = 0;
	if ((pos = rqst.find("Transfer-Encoding: chunked")) != std::string::npos)
	{
		std::cout << "Chunk header detected at pos: " << pos << std::endl;
		pos = rqst.find("\r\n\r\n");
		std::cout << "rqst:" << rqst << std::endl;
		chunk_buffer = rqst.substr(pos + 4);
//		std::cout << "chunk_buffer:" << chunk_buffer << std::endl;
		mode = CHUNKED;
		chunk_expected = 0;
		rqst = rqst.substr(0, pos + 4);
		if (chunk_buffer.empty())
		{
			std::cout << "Chunk header complete, checking for body" << std::endl;
			chunk_mode = CHUNK_COMPLETE;
		}
		else
		{
			std::cout << "Chunk body remaining, enabling parsing mode: " << std::endl;
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
		std::cout << "Len was set:" << _cLen << std::endl;
		if (rqst.substr(rqst.find("\r\n\r\n") + 4).length() >= _cLen)
		{
			rq_mode = NORMAL;
			return (true);
		}
		rq_mode = CLEN;
	}
	else if ((boundary_pos = rqst.find("boundary=")) != string::npos && (boundary_pos += 11)) // +9 == "boundary=".length, moins deux des premiers '-' +2
	{
		post_boundary = rqst.substr(boundary_pos, rqst.find_first_of('\r', boundary_pos) - boundary_pos);
		rq_mode = MULTIPART;
	}
	else
	{
		rq_mode = NORMAL;
		return (true);
	}
	return false;
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
		{
			rqst = rqst.substr(0, _cLen);
			rq_mode = NORMAL;
			return (true);
		}
	}
	return (false);
}

void client_info::is_chunked_rqst_fulfilled()	{
//	cout << YELLOW "DANS IS_CHUNKED_REQUEST_FULFILLED\n" RESET << std::endl;
	size_t	size;
	size_t	pos;
	std::string	tmp, buf;

//	pos = rqst.length();
//	if (pos > 5 && rqst.substr(pos - 5, pos) == "0\r\n\r\n")
	if (chunk_expected != 0)
	{
		std::cout << "Still more chunks to parse" << std::endl;
		if (chunk_buffer.substr(0, 2) == "\r\n")
		{
			std::cout << "End of hex size removed" << std::endl;
			chunk_buffer = chunk_buffer.substr(2);
		}
		if (chunk_buffer.length() >= chunk_expected + 2)
		{
			std::cout << "chunk_buffer[chunk_expected]-[chunk_expected + 1] = " << (int)chunk_buffer[chunk_expected] << ", "
						<< (int)chunk_buffer[chunk_expected + 1] << std::endl;
			std::cout << "chunk_buffer[chunk_expected + 2]-[chunk_expected + 3] = " << (int)chunk_buffer[chunk_expected + 2] << ", "
						<< (int)chunk_buffer[chunk_expected + 3] << std::endl;
			std::cout << "Current chunk complete !" << std::endl;
			rqst.append(chunk_buffer.substr(0, chunk_expected));
			chunk_buffer = chunk_buffer.substr(chunk_expected);
			chunk_expected = 0;
			if (chunk_buffer.substr(0, 2) == "\r\n")
			{
				std::cout << "Cleaning end of chunk marker" << std::endl;
				chunk_buffer = chunk_buffer.substr(2);
			}
			chunk_mode = CHUNK_COMPLETE;
		}
		else
		{
			std::cout << "More chunks expected : " << chunk_expected << std::endl;
			chunk_mode = CHUNK_INCOMPLETE;
		}
	}
	else
	{
		if (chunk_buffer.substr(0, 5) == "0\r\n\r\n")
		{
			std::cout << "Chunked transmission complete !" << std::endl;
			chunk_buffer.clear();
			chunk_mode = TRANSMISSION_OVER;
			return ;
		}
		std::cout << "Cleaning chunked request" << std::endl;
		std::cout << "Number of bytes in buffer : " << chunk_buffer.length() << std::endl;
		//if (chunk_buffer.length() > 2 && chunk_buffer.substr(0, 2) == "\r\n")
		//	chunk_buffer = chunk_buffer.substr(2);
		//Jumping to end of header
		buf.clear();
		pos = 0;
		while (chunk_buffer[pos] && isxdigit(chunk_buffer[pos]))
		{
			std::cout << "found an hex digit, incrementing buf: " << buf << std::endl;
			buf += chunk_buffer[pos];
			pos++;
		}
	//	std::cout << "Found chunk size delimiter: " << size << std::endl;
		if (buf.empty())
		{
			std::cout << "Error in chunked transmission !" << std::endl;
			std::cout << "chunk_buffer[0]-[1] = " << (int)chunk_buffer[0] << ", "
						<< (int)chunk_buffer[1] << std::endl;
			std::cout << "chunk_buffer[2]-[3] = " << (int)chunk_buffer[2] << ", "
					<< (int)chunk_buffer[3] << std::endl;
			chunk_mode = BAD_REQUEST;
			return ;
		}
		else
			size = std::strtoul(buf.c_str(), NULL, 16);
		std::cout << "Hex digit translated : " << size << std::endl;
		std::cout << "chunk_buffer[pos]-[pos + 1] = " << (int)chunk_buffer[pos] << ", "
					<< (int)chunk_buffer[pos + 1] << std::endl;
		std::cout << "chunk_buffer[pos + 2]-[pos + 3] = " << (int)chunk_buffer[pos + 2] << ", "
					<< (int)chunk_buffer[pos + 3] << std::endl;
		if (size == 0)
		{
			std::cout << "Read a zero, checking for end symbol" << std::endl;
			if (chunk_buffer.substr(pos, 4) == "\r\n\r\n")
			{
				std::cout << "Chunked transmission complete !" << std::endl;
				chunk_buffer.clear();
				chunk_mode = TRANSMISSION_OVER;
				return ;
			}
			else if (chunk_buffer.substr(pos, 2) == "\r\n")
			{
				std::cout << "Might be the end" << std::endl;
			}
		}
		else
		{
			chunk_buffer = chunk_buffer.substr(pos);
			if (chunk_buffer.length() >= 2)
			{
				if (chunk_buffer.substr(0, 2) == "\r\n")
				{
					std::cout << "End of hex size removed" << std::endl;
					chunk_buffer = chunk_buffer.substr(2);
				}
				else
				{
					std::cout << "Error in chunked transmission !" << std::endl;
					chunk_mode = BAD_REQUEST;
					return ;
				}
			}
			if (chunk_buffer.length() >= size + 2)
			{
				std::cout << "Received data bigger than chunk size" << std::endl;
				std::cout << "chunk_buffer[size]-[size + 1] = " << (int)chunk_buffer[size] << ", "
							<< (int)chunk_buffer[size + 1] << std::endl;
				std::cout << "chunk_buffer[size + 2]-[size + 3] = " << (int)chunk_buffer[size + 2] << ", "
							<< (int)chunk_buffer[size + 3] << std::endl;
//				std::cout << "Remaining to parse : " << chunk_buffer.substr(size) << std::endl;
				if (chunk_buffer.substr(size, 2) != "\r\n")
				{
					std::cout << "Error in chunked transmission !" << std::endl;
					chunk_mode = BAD_REQUEST;
					return ;
				}
				else
				{
					std::cout << "Chunk complete, checking for more !" << std::endl;
					rqst.append(chunk_buffer.substr(0, size));
					chunk_buffer = chunk_buffer.substr(size + 2);
					chunk_mode = CHUNK_COMPLETE;
				}
			}
			else
			{
				chunk_expected = size;
				std::cout << "Chunk incomplete, looking for more :" << chunk_expected << std::endl;
				chunk_mode = CHUNK_INCOMPLETE;
			}
		}
	}
}

void	client_info::time_reset()	{
	time(&rqst_time_start);
}

void	client_info::remove()	{
	epoll_ctl(_epoll->_epoll_fd, EPOLL_CTL_DEL, com_socket, NULL);
	if (close(com_socket))
		throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
	com_socket = -1;
}