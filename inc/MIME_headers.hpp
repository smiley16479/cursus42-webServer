#ifndef MIME_HEADERS_HPP
# define MIME_HEADERS_HPP

#include <unistd.h>
#include <cstring>
#include <iostream>

typedef struct	s_mtype	{
	const char *type;
	std::pair<const char*, const char*>	(*sub_type)(int i);
}	t_mtype;

static pair<const char*, const char*>	text(int i)	{
	const pair<const char*, const char*>	sub_type [] = {
			make_pair("css", "css"),
			make_pair("csv", "csv"),
			make_pair("htm", "html"),
			make_pair("html", "html"),
			make_pair("ics", "calendar"),
			make_pair("", "")
		};

	return (sub_type[i]);
}

static pair<const char*, const char*>	image(int i)	{
	const pair<const char*, const char*>	sub_type[] = {
			make_pair("bmp", "bmp"),
			make_pair("gif", "gif"),
			make_pair("ico", "x-icon"),
			make_pair("jpeg", "jpeg"),
			make_pair("jpg", "jpeg"),
			make_pair("png", "png"),
			make_pair("svg", "svg+xml"),
			make_pair("tif", "tiff"),
			make_pair("tiff", "tiff"),
			make_pair("webp", "webp"),
			make_pair("", "")
		};

	return (sub_type[i]);
}

static pair<const char*, const char*> audio(int i)	{
	const pair<const char*, const char*>	sub_type[] = {
			make_pair("aac", "aac"),
			make_pair("mid", "midi"),
			make_pair("midi", "midi"),
			make_pair("oga", "ogg"),
			make_pair("wav", "x-wav"),
			make_pair("weba", "webm"),
			make_pair("3gp", "3gpp"),
			make_pair("3g2", "3gpp2"),
			make_pair("", "")
		};

	return (sub_type[i]);
}

static pair<const char*, const char*>	video(int i)	{
	const pair<const char*, const char*>	sub_type[] = {
			make_pair("avi", "x-msvideo"),
			make_pair("mpeg", "mpeg"),
			make_pair("mp4", "mp4"),
			make_pair("ogv", "ogg"),
			make_pair("webm", "webm"),
			make_pair("3gp", "3gpp"),
			make_pair("3g2", "3gpp2"),
			make_pair("", "")
		};

	return (sub_type[i]);
}

static pair<const char*, const char*>	application(int i)	{
	const pair<const char*, const char*>	sub_type[] = {
			make_pair("abw", "x-abiword"),
			make_pair("arc", "octet-stream"),
			make_pair("azw", "vnd.amazon.ebook"),
			make_pair("bin", "octet-stream"),
			make_pair("bz", "x-bzip"),
			make_pair("bz2", "x-bzip2"),
			make_pair("csh", "x-csh"),
			make_pair("doc", "msword"),
			make_pair("docx", "vnd.openxmlformats-officedocument.wordprocessingml.document"),
			make_pair("eot", "vnd.ms-fontobject"),
			make_pair("epub", "epub+zip"),
			make_pair("jar", "java-archive"),
			make_pair("js", "javascript"),
			make_pair("json", "json"),
			make_pair("mpkg", "vnd.apple.installer+xml"),
			make_pair("odp", "vnd.oasis.opendocument.presentation"),
			make_pair("ods", "vnd.oasis.opendocument.spreadsheet"),
			make_pair("odt", "vnd.oasis.opendocument.text"),
			make_pair("ogx", "ogg"),
			make_pair("pdf", "pdf"),
			make_pair("ppt", "vnd.ms-powerpoint"),
			make_pair("pptx", "vnd.openxmlformats-officedocument.presentationml.presentation"),
			make_pair("rar", "x-rar-compressed"),
			make_pair("rtf", "rtf"),
			make_pair("sh", "x-sh"),
			make_pair("swf", "x-shockwave-flash"),
			make_pair("tar", "x-tar"),
			make_pair("ts", "ts"),
			make_pair("vsd", "vnd.visio"),
			make_pair("xhtml", "xhtml+xml"),
			make_pair("xls", "vnd.ms-excel"),
			make_pair("xlsx", "vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
			make_pair("xml", "xml"),
			make_pair("xul", "vnd.mozilla.xul+xml"),
			make_pair("zip", "zip"),
			make_pair("7z", "x-7z-compressed"),
			make_pair("", "")
		};

	return (sub_type[i]);
}

static pair<const char*, const char*>	font(int i)	{
	const pair<const char*, const char*>	sub_type [] = {
			make_pair("otf", "otf"),
			make_pair("ttf", "ttf"),
			make_pair("woff", "woff"),
			make_pair("woff2", "woff2"),
			make_pair("", "")
		};

	return (sub_type[i]);
}

static const t_mtype	init_type(const char *type, pair<const char*, const char*>	(*sub_type)(int))	{
	t_mtype	ret;

	ret.type = type;
	ret.sub_type = sub_type;
	return (ret);
}

static const t_mtype	type_str(int i)	{
	const t_mtype	_m_types[6] = {
			init_type("text", text),
			init_type("image", image),
			init_type("audio", audio),
			init_type("video", video),
			init_type("application", application),
			init_type("font", font)
			};

	return (_m_types[i]);
}


static int	is_mime_type(const char *ext, pair<const char*, const char*>	(*type)(int))	{
	for (size_t i = 0; strcmp(type(i).first, "") != 0; i++)
	{
	#ifdef _debug_
		std::cout << "Coucou, l'index vaut : " << i << " pour sub_type" << std::endl;
		std::cout << "Le sous type correspondant c'est : " << type(i).first << std::endl;
	#endif
		if (strcmp(ext, type(i).first) == 0)
			return (i);
	}
	return (-1);
}

static void	get_mime_type(const char *ext, int (*index)[2])	{
	for (size_t i = 0; i < 6; i++) 
	{
	#ifdef _debug_
		std::cout << "Coucou, l'index vaut : " << i << " pour le type" << std::endl;
		std::cout << "Le type correspondant c'est : " << type_str(i).type << std::endl;
	#endif
		if (((*index)[1] = is_mime_type(ext, type_str(i).sub_type)) != -1)
		{
			(*index)[0] = i;
			return ;
		}
	}
	(*index)[0] = -1;
}

std::string	mime_string(const char *ext)	{
	int	index[2];
	std::string	ret;

	get_mime_type(ext, &index);
	if (index[0] == -1 || index[1] == -1)
	{
		#ifdef _debug_
		std::cout << "MIME type not recognised" << std::endl;
		#endif
		ret = "application/octet-stream";
		return (ret);
	}
	#ifdef _debug_
	std::cout << "MIME type identified ! type index = " << index[0] << " sub_type index = " << index[1] << std::endl;
	#endif
	ret = (char*)type_str(index[0]).type;
	ret += (char*)"/";
	ret += (char*)type_str(index[0]).sub_type(index[1]).second;
	if (index[0] == 0)
		ret += (char*)"; charset=utf-8";
	return (ret);
}

#endif
