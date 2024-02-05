#include "float_math.h"
#include "ConvexBuilder.h"
#include "meshvolume.h"
#include "bestfit.h"
#include <assert.h>
#include "cd_hull.h"

#include "fitsphere.h"
#include "bestfitobb.h"

unsigned int MAXDEPTH = 8 ;
float CONCAVE_PERCENT = 1.0f ;
float MERGE_PERCENT   = 2.0f ;

CHull::CHull(const ConvexResult &result)
{
	mResult = new ConvexResult(result);
	mVolume = computeMeshVolume( result.mHullVertices, result.mHullTcount, result.mHullIndices );

	mDiagonal = getBoundingRegion( result.mHullVcount, result.mHullVertices, sizeof(float)*3, mMin, mMax );

	float dx = mMax[0] - mMin[0];
	float dy = mMax[1] - mMin[1];
	float dz = mMax[2] - mMin[2];

	dx*=0.1f; // inflate 1/10th on each edge
	dy*=0.1f; // inflate 1/10th on each edge
	dz*=0.1f; // inflate 1/10th on each edge

	mMin[0]-=dx;
	mMin[1]-=dy;
	mMin[2]-=dz;

	mMax[0]+=dx;
	mMax[1]+=dy;
	mMax[2]+=dz;


}

CHull::~CHull(void)
{
	delete mResult;
}

bool CHull::overlap(const CHull &h) const
{
	return overlapAABB(mMin,mMax, h.mMin, h.mMax );
}




ConvexBuilder::ConvexBuilder(ConvexDecompInterface *callback)
{
	mCallback = callback;
}

ConvexBuilder::~ConvexBuilder(void)
{
	int i;
	for (i=0;i<mChulls.size();i++)
	{
		CHull *cr = mChulls[i];
		delete cr;
	}
}

bool ConvexBuilder::isDuplicate(unsigned int i1,unsigned int i2,unsigned int i3,
								unsigned int ci1,unsigned int ci2,unsigned int ci3)
{
	unsigned int dcount = 0;

	assert( i1 != i2 && i1 != i3 && i2 != i3 );
	assert( ci1 != ci2 && ci1 != ci3 && ci2 != ci3 );

	if ( i1 == ci1 || i1 == ci2 || i1 == ci3 ) dcount++;
	if ( i2 == ci1 || i2 == ci2 || i2 == ci3 ) dcount++;
	if ( i3 == ci1 || i3 == ci2 || i3 == ci3 ) dcount++;

	return dcount == 3;
}

void ConvexBuilder::getMesh(const ConvexResult &cr,VertexLookup vc,UintVector &indices)
{
	unsigned int *src = cr.mHullIndices;

	for (unsigned int i=0; i<cr.mHullTcount; i++)
	{
		unsigned int i1 = *src++;
		unsigned int i2 = *src++;
		unsigned int i3 = *src++;

		const float *p1 = &cr.mHullVertices[i1*3];
		const float *p2 = &cr.mHullVertices[i2*3];
		const float *p3 = &cr.mHullVertices[i3*3];

		i1 = Vl_getIndex(vc,p1);
		i2 = Vl_getIndex(vc,p2);
		i3 = Vl_getIndex(vc,p3);

#if 0
		bool duplicate = false;

		unsigned int tcount = indices.size()/3;
		for (unsigned int j=0; j<tcount; j++)
		{
			unsigned int ci1 = indices[j*3+0];
			unsigned int ci2 = indices[j*3+1];
			unsigned int ci3 = indices[j*3+2];
			if ( isDuplicate(i1,i2,i3, ci1, ci2, ci3 ) )
			{
				duplicate = true;
				break;
			}
		}

		if ( !duplicate )
		{
			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i3);
		}
#endif

	}
}

CHull * ConvexBuilder::canMerge(CHull *a,CHull *b)
{

	if ( !a->overlap(*b) ) return 0; // if their AABB's (with a little slop) don't overlap, then return.

	CHull *ret = 0;

	// ok..we are going to combine both meshes into a single mesh
	// and then we are going to compute the concavity...

	VertexLookup vc = Vl_createVertexLookup();

	UintVector indices;

	getMesh( *a->mResult, vc, indices );
	getMesh( *b->mResult, vc, indices );

	unsigned int vcount = Vl_getVcount(vc);
	const float *vertices = Vl_getVertices(vc);
	unsigned int tcount = indices.size()/3;
	
	//don't do anything if hull is empty
	if (!tcount)
	{
		Vl_releaseVertexLookup (vc);
		return 0;
	}

	HullResult hresult;
	HullLibrary hl;
	HullDesc   desc;

	desc.SetHullFlag(QF_TRIANGLES);

	desc.mVcount       = vcount;
	desc.mVertices     = vertices;
	desc.mVertexStride = sizeof(float)*3;

	HullError hret = hl.CreateConvexHull(desc,hresult);

	if ( hret == QE_OK )
	{

		float combineVolume  = computeMeshVolume( hresult.mOutputVertices, hresult.mNumFaces, hresult.mIndices );
		float sumVolume      = a->mVolume + b->mVolume;

		float percent = (sumVolume*100) / combineVolume;
		if ( percent >= (100.0f-MERGE_PERCENT) )
		{
			ConvexResult cr(hresult.mNumOutputVertices, hresult.mOutputVertices, hresult.mNumFaces, hresult.mIndices);
			ret = new CHull(cr);
		}
	}


	Vl_releaseVertexLookup(vc);

	return ret;
}

bool ConvexBuilder::combineHulls(void)
{

	bool combine = false;

	sortChulls(mChulls); // sort the convex hulls, largest volume to least...

	CHullVector output; // the output hulls...


	int i;

	for (i=0;i<mChulls.size() && !combine; ++i)
	{
		CHull *cr = mChulls[i];

		int j;
		for (j=0;j<mChulls.size();j++)
		{
			CHull *match = mChulls[j];

			if ( cr != match ) // don't try to merge a hull with itself, that be stoopid
			{

				CHull *merge = canMerge(cr,match); // if we can merge these two....

				if ( merge )
				{

					output.push_back(merge);


					++i;
					while ( i != mChulls.size() )
					{
						CHull *cr = mChulls[i];
						if ( cr != match )
						{
							output.push_back(cr);
						}
						i++;
					}

					delete cr;
					delete match;
					combine = true;
					break;
				}
			}
		}

		if ( combine )
		{
			break;
		}
		else
		{
			output.push_back(cr);
		}

	}

	if ( combine )
	{
		mChulls.clear();
		mChulls = output;
		output.clear();
	}


	return combine;
}

unsigned int ConvexBuilder::process(const DecompDesc &desc)
{

	unsigned int ret = 0;

	MAXDEPTH        = desc.mDepth;
	CONCAVE_PERCENT = desc.mCpercent;
	MERGE_PERCENT   = desc.mPpercent;


	calcConvexDecomposition(desc.mVcount, desc.mVertices, desc.mTcount, desc.mIndices,this,0,0);


	while ( combineHulls() ); // keep combinging hulls until I can't combine any more...

	int i;
	for (i=0;i<mChulls.size();i++)
	{
		CHull *cr = mChulls[i];

		// before we hand it back to the application, we need to regenerate the hull based on the
		// limits given by the user.

		const ConvexResult &c = *cr->mResult; // the high resolution hull...

		HullResult result;
		HullLibrary hl;
		HullDesc   hdesc;

		hdesc.SetHullFlag(QF_TRIANGLES);

		hdesc.mVcount       = c.mHullVcount;
		hdesc.mVertices     = c.mHullVertices;
		hdesc.mVertexStride = sizeof(float)*3;
		hdesc.mMaxVertices  = desc.mMaxVertices; // maximum number of vertices allowed in the output

		if ( desc.mSkinWidth  )
		{
			hdesc.mSkinWidth = desc.mSkinWidth;
			hdesc.SetHullFlag(QF_SKIN_WIDTH); // do skin width computation.
		}

		HullError ret = hl.CreateConvexHull(hdesc,result);

		if ( ret == QE_OK )
		{
			ConvexResult r(result.mNumOutputVertices, result.mOutputVertices, result.mNumFaces, result.mIndices);

			r.mHullVolume = computeMeshVolume( result.mOutputVertices, result.mNumFaces, result.mIndices ); // the volume of the hull.

			// compute the best fit OBB
			computeBestFitOBB( result.mNumOutputVertices, result.mOutputVertices, sizeof(float)*3, r.mOBBSides, r.mOBBTransform );

			r.mOBBVolume = r.mOBBSides[0] * r.mOBBSides[1] *r.mOBBSides[2]; // compute the OBB volume.

			fm_getTranslation( r.mOBBTransform, r.mOBBCenter );      // get the translation component of the 4x4 matrix.

			fm_matrixToQuat( r.mOBBTransform, r.mOBBOrientation );   // extract the orientation as a quaternion.

			r.mSphereRadius = computeBoundingSphere( result.mNumOutputVertices, result.mOutputVertices, r.mSphereCenter );
			r.mSphereVolume = fm_sphereVolume( r.mSphereRadius );


			mCallback->ConvexDecompResult(r);
		}

		hl.ReleaseResult (result);


		delete cr;
	}

	ret = mChulls.size();

	mChulls.clear();

	return ret;
}


void ConvexBuilder::ConvexDebugTri(const float *p1,const float *p2,const float *p3,unsigned int color)
{
	mCallback->ConvexDebugTri(p1,p2,p3,color);
}

void ConvexBuilder::ConvexDebugOBB(const float *sides, const float *matrix,unsigned int color)
{
	mCallback->ConvexDebugOBB(sides,matrix,color);
}
void ConvexBuilder::ConvexDebugPoint(const float *p,float dist,unsigned int color)
{
	mCallback->ConvexDebugPoint(p,dist,color);
}

void ConvexBuilder::ConvexDebugBound(const float *bmin,const float *bmax,unsigned int color)
{
	mCallback->ConvexDebugBound(bmin,bmax,color);
}

void ConvexBuilder::ConvexDecompResult(ConvexResult &result)
{
	CHull *ch = new CHull(result);
	mChulls.push_back(ch);
}

void ConvexBuilder::sortChulls(CHullVector &hulls)
{
	hulls.quickSort(CHullSort());
	//hulls.heapSort(CHullSort());
}


