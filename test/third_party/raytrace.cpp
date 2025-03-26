// sphere flake bvh raytracer (c) 2005, thierry berger-perrin <tbptbp@gmail.com>
// this code is released under the GNU Public License.
// Emscripten changes: stdlib.h, some printf stuff, SIZE

#include <cmath>       // see http://ompf.org/ray/sphereflake/
#include <iostream>    // compile with ie g++ -O2 -ffast-math sphereflake.cc
#include <stdio.h>
#include <stdlib.h>

//#include "emscripten.h"

#define GIMME_SHADOWS  // usage: ./sphereflake [lvl=6] >pix.ppm

enum { childs = 9, ss= 2, ss_sqr = ss*ss }; /* not really tweakable anymore */
static const double infinity = 1./0, epsilon = 1e-12;

struct v_t{ double x,y,z;v_t(){}
	v_t(const double a,const double b,const double c):x(a),y(b),z(c){}
	v_t operator+(const v_t&v)const{return v_t(x+v.x,y+v.y,z+v.z);}
	v_t operator-(const v_t&v)const{return v_t(x-v.x,y-v.y,z-v.z);}
	v_t operator-()const{return v_t(-x,-y,-z);}
	v_t operator*(const double d)const{return v_t(x*d,y*d,z*d);}
	v_t cross(const v_t&v)const{return v_t(y*v.z-z*v.y,z*v.x-x*v.z,x*v.y-y*v.x);}
	v_t norm()const{return*this*(1./sqrt(magsqr()));}
	double dot(const v_t&v)const{return x*v.x+y*v.y+z*v.z;}
	double magsqr()const{return dot(*this);}
};

//static const v_t light(v_t(0.5,-.95,1.775).norm()); /*pick one*/
static const v_t light(v_t(-0.5,-.65,.9).norm()); /*fiat lux*/

struct ray_t{
	v_t o,d;
	ray_t(const v_t&v):o(v){}
	 ray_t(const v_t&v,const v_t&w):o(v),d(w){}
};
struct hit_t {
	v_t n;
	double t;
	hit_t():n(v_t(0,0,0)),t(infinity){}
};

struct sphere_t{ 
	v_t o;
	double r;
	sphere_t(){}
	sphere_t(const v_t&v,double d):o(v),r(d){}
	v_t get_normal(const v_t&v)const{return(v-o)*(1./r);}
	double intersect(const ray_t&ray)const{
		const v_t v(o-ray.o); const double b=ray.d.dot(v),disc=b*b-v.magsqr()+r*r;
		if(disc < 0.)
			return infinity; /*branch away from the square root*/
		const double d=sqrt(disc), t2=b+d, t1=b-d; /*cond. move*/
		if(t2 < 0.)
			return infinity;
		else
			return(t1 > 0.? t1 : t2);
	}
};

struct node_t; 
static node_t *pool=0, *end=0;

struct node_t { /*a bvh in array form+skip for navigation.*/
	sphere_t bound,leaf;
	long diff;/*far from optimal*/
	node_t(){} node_t(const sphere_t&b,const sphere_t&l,const long jump) :bound(b),leaf(l),diff(jump){}
	template<bool shadow> static void intersect(const ray_t &ray,hit_t &hit){
		const node_t*p=pool;
		while(p < end) {
			if(p->bound.intersect(ray)>=hit.t) /*missed bound*/
				p+=p->diff; /*skip subtree*/
			else{
				const double t=p->leaf.intersect(ray);
				if(t < hit.t) { /*if hit, update, then break for shadows*/
					 hit.t=t;
					if(shadow) break;
					hit.n=p->leaf.get_normal(ray.o+ray.d*t);
				}
				++p; /*next!*/
			}
		}
	}
};

static double ray_trace(const node_t*const scene,const ray_t&ray) {
	hit_t hit;
	scene->intersect<false>(ray,hit);// trace primary
	const double diffuse = hit.t==infinity ? 0. : -hit.n.dot(light);
	#ifdef GIMME_SHADOWS
		if (diffuse <= 0.)
			return 0.;
		const ray_t sray(ray.o+(ray.d*hit.t)+(hit.n*epsilon),-light);
		hit_t shit;
		scene->intersect<true>(sray,shit);// trace shadow
		return shit.t==infinity ? diffuse : 0.;
	#else
		return diffuse > 0. ? diffuse : 0.;
	#endif
}

static const double grid[ss_sqr][2]={ /*our rotated grid*/
	{-3/3.,-1/3.},{+1/3.,-3/3.},
	{-1/3.,+3/3.},{+3/3.,+1/3.}
};
static void trace_rgss(const int width,const int height) {
	const double w=width,h=height,rcp=1/double(ss),scale=256./double(ss_sqr);
	ray_t ray(v_t(0,0,-4.5)); /* eye, looking into Z */
	v_t rgss[ss_sqr];
	for(int i=0;i<ss_sqr;++i) /*precomp.*/ {
		rgss[i]=v_t(grid[i][0]*rcp-w/2.,grid[i][1]*rcp-h/2.,0);
	}
	v_t scan(0,w-1,std::max(w,h)); /*scan line*/
	for(int i=height;i;--i) {
    int lineMean = 0;
		for(int j=width;j;--j) {
			double g=0;
			for(int idx=0;idx < ss_sqr;++idx){ /*AA*/
				ray.d=(scan+rgss[idx]).norm();
				g+=ray_trace(pool,ray); /*trace*/
			}
  		lineMean += int(scale*g);
			scan.x+=1; /*next pixel*/
		}
    printf("%d : %d\n", i, lineMean/width);
		scan.x=0;scan.y-=1; /*next line*/
	}
}
	
struct basis_t{ /* bogus and compact, exactly what we need */
	v_t up,b1,b2;
	basis_t(const v_t&v){ const v_t n(v.norm());
		if ((n.x*n.x !=1.)&(n.y*n.y !=1.)&(n.z*n.z !=1.)) {/*cough*/
			b1=n;
			if(n.y*n.y>n.x*n.x) {
				if(n.y*n.y>n.z*n.z)
					b1.y=-b1.y;
				else b1.z=-b1.z;
			}
			else if(n.z*n.z > n.x*n.x)
				b1.z=-b1.z;
			else b1.x=-b1.x;
		}
		else
			b1=v_t(n.z,n.x,n.y);/*leaves some cases out,dodge them*/
		
		up=n;
		b2=up.cross(b1);
		b1=up.cross(b2);
	}
};

static node_t *create(node_t*n,const int lvl,int dist,v_t c,v_t d,double r) {
	n = 1 + new (n) node_t(sphere_t(c,2.*r),sphere_t(c,r), lvl > 1 ? dist : 1);
	if (lvl <= 1)
		return n; /*if not at the bottom, recurse a bit more*/
	dist=std::max((dist-childs)/childs,1); const basis_t b(d); 
	const double nr=r*1/3.,daL=2.*M_PI/6.,daU=2.*M_PI/3.; double a=0;
	for(int i=0;i<6;++i){ /*lower ring*/
		const v_t ndir((d*-.2+b.b1*sin(a)+b.b2*cos(a)).norm()); /*transcendentals?!*/
		n=create(n,lvl-1,dist,c+ndir*(r+nr),ndir,nr);
		a+=daL;
	}
	a-=daL/3.;/*tweak*/
	for(int i=0;i<3;++i){ /*upper ring*/
		const v_t ndir((d*+.6+b.b1*sin(a)+b.b2*cos(a)).norm());
		n=create(n,lvl-1,dist,c+ndir*(r+nr),ndir,nr); a+=daU;
	}
	return n;
}

int main(int argc,char*argv[]){
	const int lvl=atoi(argv[1]);
	const int size=atoi(argv[2]);
	int count=childs, dec=lvl;
	while(--dec > 1) count=(count*childs)+childs;
	++count;
	pool=new node_t[count];  /* raw */
	end=pool+count;
	create(pool,lvl,count,v_t(0,0,0),v_t(+.25,+1,-.5).norm(),1.); /* cooked */
	printf("P2\n%d %d\n%d\n", size, size, size); // std::cout << "P2\n" << size << " " << size << "\n" << size << "\n";
	trace_rgss(size, size); /* served */
	return 0;
}
