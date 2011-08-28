/**
 * Copyright (C) 2011 cxxjoe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#ifndef _KOPF_LISCHINSKI_H
#define _KOPF_LISCHINSKI_H

#include "nfo_renderer.h"
#include "boost/graph/adjacency_matrix.hpp"


class CKopfLischinskiNFORenderer : public CNFORenderer
{
public:
	CKopfLischinskiNFORenderer();

	virtual bool AssignNFO(const PNFOData& a_nfo);
	bool Render();

protected:
	struct TVertexColor { typedef boost::vertex_property_tag kind; };

	typedef boost::property<TVertexColor, unsigned short> TVertexColorProperty;
	typedef boost::adjacency_matrix<boost::undirectedS, TVertexColorProperty> TGraph;
	typedef boost::shared_ptr<TGraph> PGraph;

	TwoDimVector<unsigned short>* m_intermediate;
	size_t m_pxPerRow;
	PGraph m_graph;

	void Transform_GridToBitmap(bool a_highRes);
	void Transform_BitmapToGraph();
};

#endif /* !_KOPF_LISCHINSKI_H */
