#ifndef CLIENT_INFO_HPP
# define CLIENT_INFO_HPP

#include <sys/socket.h>	// pour socket()
#include <cstring>
#include "request_handler.hpp"
#include "struct_webserv.hpp"

class client_info {
	public:
		int			com_socket;
		int			loc_fd;
		size_t		mode;
		std::string	resp;
		std::string rqst;
		std::string buf;
		std::string post_boundary;
		time_t		rqst_time_start;
		size_t		_cLen;
		int			time_out;
		struct struct_epoll* _epoll;

		void	fd_in(request_handler& header);
		void	fd_out(request_handler& header);
		void	compute(request_handler& header);
		void	recv_handler(request_handler& header);
		void	read_handler(request_handler& header);
		void	write_handler(request_handler& header);
		void	send_handler(request_handler& header);
		void	cgi_resp_handler(request_handler& header);

		bool	is_request_fulfilled();
		bool	is_post_rqst_fulfilled();
		bool	is_chunked_rqst_fulfilled();

		void 	time_reset();
		void 	remove();
};

typedef void	(client_info::*t_func)(request_handler&);

#endif
