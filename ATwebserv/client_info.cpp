#include "client_info.hpp"

void	client_info::fd_in(request_handler& header)	{
	if (mode == COMPUTE)
		return (compute(header));
	else if (mode == RECV)
		return (recv_handler(header));
	return ;
}

void	client_info::fd_out(request_handler& header)	{
	if (mode == COMPUTE)
		return (compute(header));
	else if (mode == READ)
		return (read_handler(header));
	else if (mode == WRITE)
		return (write_handler(header));
	else if (mode == SEND)
		return (send_handler(header));
//	else if (mode == CGI_OUT)
//		return (cgi_resp_handler(header));
	return ;
}

void	client_info::compute(request_handler& header)	{
	int	ret;

	ret = is_request_fulfilled();
	if (ret == true)
	{
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
				rqst = header.get_response();
				resp = header.get_body();
			}
			return ;
		}
		mode = SEND;
	}
//	else
//		remove();
	//SHOULD SEND A BAD REQUEST RESP
	//OR FIGURE WHETHER THE MSG IS CHUNKED
}

void	client_info::recv_handler(request_handler& header)	{
	(void)header;
	int		recv_bytes;
	char	buf[MAX_LEN];

	recv_bytes = recv(com_socket, &buf, MAX_LEN, MSG_NOSIGNAL);
	if (recv_bytes == -1)
	{
		std::cout << "RECV ERROR" << std::endl;
		remove();
		return ;
	}
	else if (recv_bytes == 0)
	{
		if (is_request_fulfilled())
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
		std::cout << "MSG TOO LONG" << std::endl;
		std::cout << "(len is " << resp.length() << ")" << std::endl;
		tmp = resp.substr(MAX_LEN);
		std::cout << "MSG RECUT" << std::endl;
	}
	wrote_bytes = write(loc_fd, resp.c_str(), MAX_LEN);
	if (wrote_bytes == -1)
	{
		std::cout << "WRITE ERROR" << std::endl;
		close(loc_fd);
		resp.clear();
		mode = SEND;
		return ;
	}
	else if (wrote_bytes == 0)
	{
		std::cout << "WRITE EOF" << std::endl;
		mode = SEND;
	}
	else if (wrote_bytes < MAX_LEN)
	{
		std::cout << "WRITE MSG END" << std::endl;
		mode = SEND;
	}
	resp = tmp;
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
		std::cout << "MSG RECUT" << std::endl;
	}
	sent_bytes = send(com_socket, resp.c_str(), MAX_LEN, MSG_NOSIGNAL);
	if (sent_bytes == -1)
	{
		std::cout << "SEND ERROR" << std::endl;
		remove();
		return ;
	}
	else if (sent_bytes == 0)
	{
		std::cout << "SEND EOF" << std::endl;
		mode = READ;
	}
	else if (sent_bytes < MAX_LEN)
	{
		std::cout << "SEND MSG END" << std::endl;
		mode = READ;
	}
	resp = tmp;
}




bool client_info::is_request_fulfilled()
{
	// cout << BLUE "DANS IS_REQUEST_FULFILLED, size of current reqst : " << clients[client_fd].rqst.length() <<"\n" RESET;
		
	if (rqst.substr(0, 4) == "POST")
	{
		return is_post_rqst_fulfilled(); // HANDLE RECOGNITION OF ENDED POST RQST
	}
	else if (rqst.substr(0, 5) == "CHUNK")
	{
		return is_chunked_rqst_fulfilled(); // HANDLE RECOGNITION OF ENDED POST RQST
	}

	// for (size_t i = clients[client_fd].rqst.length() - 4; i < clients[client_fd].rqst.length(); ++i)
	// 	cout << "clients[client_fd][i] :[" << clients[client_fd].rqst[i] << "]\n";

	size_t len = rqst.size();
	if (len >= 4 && rqst.substr(len - 4, len) == "\r\n\r\n") // SUREMENT UNE MAUVAISE FAÇON DE LE FAIRE
		return true ;
	return false ;
}

bool client_info::is_post_rqst_fulfilled()
{
	size_t				pos;
	std::string			tmp;
	std::stringstream	ss;

	cout << YELLOW "DANS IS_POST_REQUEST_FULFILLED\n" RESET << std::endl;
	_cLen = 0;
	if ((pos = rqst.find("Transfer-Encoding: chunked")) != std::string::npos)
	{
		rqst.replace(0, 4, "CHUNK", 5);
		std::cout << "chunks:" << pos << std::endl;
	}
	else if ((pos = rqst.find("\r\n\r\n")) != string::npos)
	{
		if ((pos = rqst.find("Content-Length: ")) != std::string::npos)
		{
			tmp = rqst.substr(pos + strlen("Content-Length: "));
			if ((pos = tmp.find("\r\n")) != std::string::npos)
				tmp = tmp.substr(0, pos);
			ss.str(tmp);
			ss >> _cLen;
			std::cout << "Len was set:" << _cLen << std::endl;
			if (rqst.substr(rqst.find("\r\n\r\n") + 4).length() >= _cLen)
			{
				return (true);
			}
			if (_cLen > MAX_LEN)
			{
				rqst.replace(0, 4, "CHUNK");
				std::cout << "len requires chunking" << std::endl;
			}
		}
		else
			return (true);
	}
	if (post_boundary.empty()) {
		size_t boundary_pos;

		if ((boundary_pos = rqst.find("boundary=")) != string::npos && (boundary_pos += 11)) // +9 == "boundary=".length, moins deux des premiers '-' +2
			post_boundary = rqst.substr(boundary_pos, rqst.find_first_of('\r', boundary_pos) - boundary_pos);
			// cout << "boundary_pos : "<< boundary_pos << ", boundary_pos of fisrt \\r in boundary : " << clients[client_fd].rqst.find_first_of('\r', boundary_pos) << endl;
			// cout << "boundary : [" << clients[client_fd].post_boundary << "]" RESET << endl;
			// cout << "boundary : [" << clients[client_fd].post_boundary + "--\r\n" << "]" RESET << endl;
	}
	else
	{
		if ((pos = rqst.rfind(post_boundary)) != string::npos)
		{
			tmp = rqst.substr(0, pos + strlen(post_boundary.c_str()));
			if (_cLen > tmp.length())
			{
				return (false);
			}
			else
			{
				if (rqst.substr(0, 5) == "CHUNK")
					rqst.replace(0, 5, "POST", 4);
				return (true);
			}
		}
	}
	return false;
}

bool client_info::is_chunked_rqst_fulfilled()
{
//	cout << YELLOW "DANS IS_CHUNKED_REQUEST_FULFILLED\n" RESET << std::endl;
	size_t	pos;
	std::string	tmp;
	std::stringstream	ss;

	if (post_boundary.empty()) {
		size_t boundary_pos;

		if ((boundary_pos = rqst.find("boundary=")) != string::npos && (boundary_pos += 11)) // +9 == "boundary=".length, moins deux des premiers '-' +2
			post_boundary = rqst.substr(boundary_pos, rqst.find_first_of('\r', boundary_pos) - boundary_pos);
	}
	if ((pos = rqst.find(post_boundary + "--")) != std::string::npos)
	{
		if (rqst.substr(0, 5) == "CHUNK")
			rqst.replace(0, 5, "POST");
		return (true);
	}
	if (_cLen != 0)
	{
		if (_cLen <= rqst.length())
		{
			if (_cLen < tmp.length())
				rqst = rqst.substr(0, _cLen);
			if (rqst.substr(0, 5) == "CHUNK")
				rqst.replace(0, 5, "POST");
			return (true);
		}
	}
	else
	{
		if ((pos = rqst.find("Content-Length: ")) != std::string::npos)
		{
			tmp = rqst.substr(pos + strlen("Content-Length: "));
			if ((pos = tmp.find("\r\n")) != std::string::npos)
				tmp = tmp.substr(0, pos);
			ss.str(tmp);
			ss >> _cLen;
			if (_cLen < rqst.length())
				return (true);
			if (_cLen > MAX_LEN)
			{
				rqst.replace(0, 4, "CHUNK");
				std::cout << "len requires chunking" << std::endl;
			}
		}
	}
//	pos = rqst.length();
//	if (pos > 5 && rqst.substr(pos - 5, pos) == "0\r\n\r\n")
	if (rqst.rfind("0\r\n\r\n") != std::string::npos)
	{
		if (rqst.substr(0, 5) == "CHUNK")
			rqst.replace(0, 5, "POST");
		return (true);
	}
//	std::cout << rqst << std::endl;
	return (false);
}

void client_info::remove()
{
	epoll_ctl(_epoll->_epoll_fd, EPOLL_CTL_DEL, com_socket, NULL);
	if (close(com_socket))
		throw std::runtime_error("CLOSE FAILLED (client_handler::remove)");
}