// Halide Template Matching v2 AOT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace Halide;



void createAOT(int i){
	Var x("x"), y("y"), x_outer("x_outer"), y_outer("y_outer"), x_inner("x_inner"), y_inner("y_inner"), tile_index("tile_index");
	ImageParam source(type_of<float>(), 2,"source");
	ImageParam templ(type_of<float>(), 2, "templ");
	Func  limit("limit"), score("score");
	limit = BoundaryConditions::constant_exterior(source, 1.0f);

	RDom matchDom(0, templ.width(), 0, templ.height());
	RDom searchDom(0, source.width() - templ.width(), 0, source.height() - templ.height());

	score(x, y) = sum(matchDom, pow(templ(matchDom.x, matchDom.y) - limit(x + matchDom.x, y + matchDom.y), 2)) / (templ.width() * templ.height());

	Target t = get_host_target();
		switch (i) {
		case 0:
			score.compile_to_static_library("sqdiff_unoptimized", { source,templ },t);
			break;
		case 1:
			score
				.tile(x, y, x_outer, y_outer, x_inner, y_inner, 64, 64)
				.fuse(x_outer, y_outer, tile_index)
				.vectorize(x_inner, 4)
				.parallel(tile_index)
				.compute_root();
			score.compile_to_static_library("sqdiff_tiling", { source,templ },t);
			break;
		case 2:
			score
				.vectorize(x, 16)
				.parallel(y)
				.compute_root();
			score.compile_to_static_library("sqdiff_vectorize", { source,templ },t);
		}
}


void createAOTccorr(int i ) {
	Var x("x"), y("y"), x_outer("x_outer"), y_outer("y_outer"), x_inner("x_inner"), y_inner("y_inner"), tile_index("tile_index");

	ImageParam source(type_of<float>(), 2, "source");
	ImageParam templ(type_of<float>(), 2, "templ");
	//Param<int> typeOfOptimization;

	Func limit("limit");

	limit = BoundaryConditions::constant_exterior(source, 0.0f);

	Func score("score");

	RDom matchDom(0, templ.width(), 0, templ.height());
	RDom searchDom(0, source.width() - templ.width(), 0, source.height() - templ.height());


	score(x, y) = sum(matchDom, templ(matchDom.x, matchDom.y) * limit(x + matchDom.x, y + matchDom.y)) / sqrt(sum(matchDom, pow(templ(matchDom.x, matchDom.y), 2)) * sum(matchDom, pow(limit(x + matchDom.x, y + matchDom.y), 2)));

/*	score
		.vectorize(x, 16)
		.parallel(y)
		.compute_inline();*/

	Target t = get_host_target();

	t.set_feature(Target::NoRuntime, true);

//	for (int i = 0; i < 3; i++) {
	
		switch (i) {
		case 0:
			score.compile_to_static_library("ccorr_unoptimized", { source,templ });
			break;
		case 1:
			score
				.tile(x, y, x_outer, y_outer, x_inner, y_inner, 64, 64)
				.fuse(x_outer, y_outer, tile_index)
				.vectorize(x_inner, 4)
				.parallel(tile_index)
				.compute_root();
			score.compile_to_static_library("ccorr_tiling", { source,templ });
			break;
		case 2:
			score
				.vectorize(x, 16)
				.parallel(y)
				.compute_root();
			score.compile_to_static_library("ccorr_vectorize", { source,templ });
		}
	//}





	//score.compile_to_static_library("template_matching_ccorr", { source,templ,typeOfOptimization });

}



int main()
{
	//for (int i = 0; i < 3; i++) {
		createAOT(2);
		//createAOTccorr(i);
	//}


		
    return 0;
}

