/*
* Copyright 2016 Google Inc. All Rights Reserved.

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <cstdio>
#include <list>
#include <algorithm>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <sstream>
#include <iterator>

#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif /* N WIN32 */

typedef float decimal_t;

typedef std::tuple<decimal_t, decimal_t, decimal_t> vec3_t;
typedef std::tuple<decimal_t, decimal_t> vec2_t;

const int vertices_per_face = 3;

struct point_t {
	int vertex, normal, texcoord;

	inline bool operator==(const point_t & point) const {
		return vertex == point.vertex && normal == point.normal && texcoord == point.texcoord;
	}
};

struct point_hasher_t {
	inline std::size_t operator() (const point_t & point) const {
		return std::size_t(point.vertex << 16 | point.normal << 8 | point.texcoord);
	}
};

struct face_t {
	point_t points[vertices_per_face];
};

template<typename VecT>
VecT parse_line(const char *fmt, const char *line) {
	throw std::runtime_error("");
}

template<>
vec3_t parse_line<vec3_t>(const char *fmt, const char *line) {
	vec3_t vec(decimal_t(0), decimal_t(0), decimal_t(0));

	if (sscanf(line, fmt, &std::get<0>(vec), &std::get<1>(vec), &std::get<2>(vec)) != 3)
		throw std::runtime_error("Failed to parse vec3 line.");
	return vec;
}

template<>
vec2_t parse_line<vec2_t>(const char *fmt, const char *line) {
	vec2_t vec(decimal_t(0), decimal_t(0));

	if (sscanf(line, fmt, &std::get<0>(vec), &std::get<1>(vec)) != 2)
		throw std::runtime_error("Failed to parse vec2 line.");
	return vec;
}

template<>
face_t parse_line<face_t>(const char *fmt, const char *line) {
	face_t face;

	if (sscanf(line, fmt,
		&face.points[0].vertex, &face.points[0].texcoord, &face.points[0].normal,
		&face.points[1].vertex, &face.points[1].texcoord, &face.points[1].normal,
		&face.points[2].vertex, &face.points[2].texcoord, &face.points[2].normal) != 9)
		throw std::runtime_error("Failed to parse face line.");
	return face;
}

struct out_vector_t {
	const vec3_t vertex, normal;
	const vec2_t texcoord;

	inline out_vector_t(const vec3_t & vertex, const vec3_t & normal, const vec2_t & texcoord) : vertex(vertex), normal(normal), texcoord(texcoord) {}
};

/*
* This is a debugging tool, writes our data back as an obj that can be read to validate the functionality.
* Note that this obj will be expanded in the same way our gl format is. In other words, it's an awful obj file.
*/

void write_debug_obj(const char *filename, const std::vector<int> & out_indices, const std::vector<out_vector_t> & out_vectors) {
	std::fstream ofile(filename, std::ios_base::out);

	ofile << "o mesh" << std::endl << std::endl;

	for (const out_vector_t & vector : out_vectors)
		ofile << "v " << std::get<0>(vector.vertex) << ' ' << std::get<1>(vector.vertex) << ' ' << std::get<2>(vector.vertex) << std::endl;

	ofile << std::endl;

	for (const out_vector_t & vector : out_vectors)
		ofile << "vn " << std::get<0>(vector.normal) << ' ' << std::get<1>(vector.normal) << ' ' << std::get<2>(vector.normal) << std::endl;

	ofile << std::endl;

	for (const out_vector_t & vector : out_vectors)
		ofile << "vt " << std::get<0>(vector.texcoord) << ' ' << std::get<1>(vector.texcoord) << std::endl;

	ofile << std::endl << "g mesh" << std::endl << std::endl;

	assert(out_indices.size() % 3 == 0);
	for (size_t f = 0; f < out_indices.size() / 3; ++f) {
		ofile << "f " << (out_indices[f * 3 + 0] + 1) << '/' << (out_indices[f * 3 + 0] + 1) << '/' << (out_indices[f * 3 + 0] + 1) << ' '
			<< (out_indices[f * 3 + 1] + 1) << '/' << (out_indices[f * 3 + 1] + 1) << '/' << (out_indices[f * 3 + 1] + 1) << ' '
			<< (out_indices[f * 3 + 2] + 1) << '/' << (out_indices[f * 3 + 2] + 1) << '/' << (out_indices[f * 3 + 2] + 1) << std::endl;
	}
}

template<typename StreamT, typename IteratorT>
void join(StreamT &stream, const IteratorT &begin, const IteratorT &end, const char *separator) {
	if (begin != end) {
		stream << *begin;
		for (IteratorT it = begin + 1; it != end; ++it) {
			stream << separator << *it;
		}
	}
}

template<typename IteratorT, typename FunctorT>
void join(std::ostream &stream, const IteratorT &begin, const IteratorT &end, FunctorT functor, const char *separator) {
	if (begin != end) {
		stream << functor(*begin);
		for (IteratorT it = begin + 1; it != end; ++it) {
			stream << separator << functor(*it);
		}
	}
}

int main(int argc, const char **argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <input.obj> <output.cpp>" << std::endl;
		return 0;
	}

	std::fstream file(argv[1], std::ios_base::in);
	std::fstream ofile(argv[2], std::ios_base::out | std::ios_base::binary);

	typedef std::vector<point_t> point_container_t;

	point_container_t indices;
	std::vector<vec3_t> vertices, normals;
	std::vector<vec2_t> texcoords;

	while (file.good()) {
		std::string line;

		getline(file, line);

		switch (tolower(line[0])) {
		case 'v':
			switch (tolower(line[1])) {
			case 'n':
				normals.push_back(parse_line<vec3_t>("vn %f %f %f", line.c_str()));
				break;
			case ' ':
				vertices.push_back(parse_line<vec3_t>("v %f %f %f", line.c_str()));
				break;
			case 't':
				texcoords.push_back(parse_line<vec2_t>("vt %f %f", line.c_str()));
				break;
			}
			break;
		case 'f':
			face_t face(parse_line<face_t>("f %d/%d/%d %d/%d/%d %d/%d/%d", line.c_str()));
			for (int i = 0; i < vertices_per_face; ++i) {
				--face.points[i].vertex;
				--face.points[i].normal;
				--face.points[i].texcoord;
				indices.push_back(face.points[i]);
			}
			break;
		}
	}

	file.close();

	// Now, we have the file, but the arrangement of the data is not optimal for OpenGL.

	typedef std::unordered_map<point_t, std::vector<point_container_t::const_iterator>, point_hasher_t> unique_points_t;
	unique_points_t unique_points;

	for (point_container_t::const_iterator it(indices.begin()); it != indices.end(); ++it)
		unique_points[*it].push_back(it);

	// Now we have a list of unique points with their offsets in the original indices array of points.

	// Allocate a new indices array of the same length as the original one.

	// Extract the data the unique points are referring to (vertex, normal and texcoord)
	// push that on a vector. When doing this, take the list of indices refering to this point and set their indices to the last element of this data array.

	std::vector<int> out_indices(indices.size(), -1);

	typedef std::vector<out_vector_t> out_vector_container_t;
	out_vector_container_t out_vectors;
	out_vectors.reserve(unique_points.size());

	for (const unique_points_t::value_type & point : unique_points) {
		const size_t vector_offset(out_vectors.size());
		out_vectors.emplace_back(vertices[point.first.vertex], normals[point.first.normal], texcoords[point.first.texcoord]);

		// The indices stored along with this point all should reference to vector_offset.
		for (const point_container_t::const_iterator & it : point.second) {
			const size_t index(std::distance(indices.cbegin(), it));

			out_indices[index] = int(vector_offset);
		}
	}

	std::cerr << "Conversion extended vertices count " << vertices.size() << ", normals count " << normals.size() << " and texcoord count " << texcoords.size() << " to " << out_vectors.size() << std::endl
		<< "Indices count " << out_indices.size() << std::endl;

	for (const int & index : out_indices)
		assert(index != -1);
	assert(!out_indices.empty());

	const char *array_type(out_vectors.size() > 0x10000 ? "const_uint_array" : 
	#if 0
		(out_vectors.size() > 0x100 ? "const_ushort_array" : "const_ubyte_array"));
	#else
	 "const_ushort_array");
	#endif

	ofile << "#include <data/types.h>" << std::endl << std::endl
			<< "data::" << array_type << " indices {" << std::endl;
	join(ofile, out_indices.begin(), out_indices.end(), ", ");
	ofile << std::endl
		  << "};"
		  << std::endl
		  << std::endl;

	ofile << "data::const_vec3_array vertices {" << std::endl;
	join(ofile, out_vectors.begin(), out_vectors.end(), [](const out_vector_t & vector) {
		std::stringstream ss;
		ss << " { "
		   << std::to_string(std::get<0>(vector.vertex))
		   << "f, "
		   << std::to_string(std::get<1>(vector.vertex))
		   << "f, "
		   << std::to_string(std::get<2>(vector.vertex))
		   << "f }";
		return ss.str();
	}, ",\n");
	ofile << std::endl << "};" << std::endl << std::endl;

	ofile << "data::const_vec3_array normals {" << std::endl;
	join(ofile, out_vectors.begin(), out_vectors.end(), [](const out_vector_t & vector) {
		std::stringstream ss;
		ss << " { "
		   << std::to_string(std::get<0>(vector.normal))
		   << "f, "
		   << std::to_string(std::get<1>(vector.normal))
		   << "f, "
		   << std::to_string(std::get<2>(vector.normal))
		   << "f }";
		return ss.str();
	}, ",\n");
	ofile << std::endl
		  << "};"
		  << std::endl
		  << std::endl;

	ofile << "data::const_vec2_array texcoords {" << std::endl;
	join(ofile, out_vectors.begin(), out_vectors.end(), [](const out_vector_t & vector) {
		std::stringstream ss;
		ss << " { "
		   << std::to_string(std::get<0>(vector.texcoord))
		   << "f, "
		   << std::to_string(std::get<1>(vector.texcoord))
		   << "f }";
		return ss.str();
	}, ",\n");
	ofile << std::endl << "};" << std::endl << std::endl;
	ofile.close();

	return 0;
}
