#include "MVCScalableTestCode.h"

MVCScalableTestCode::MVCScalableTestCode()
{
}
MVCScalableTestCode::~MVCScalableTestCode()
{
}

ErrVal
MVCScalableTestCode::Destroy()
{
	delete this;
	return Err::m_nOK;
}
ErrVal
MVCScalableTestCode::WriteUVLC( UInt uiValue )
{
	UInt uiLength = 1;
	UInt uiTemp = ++uiValue;
	while( uiTemp != 1 )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}
	RNOK( WriteCode( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableTestCode::SEICode( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableTestCode *pcScalableTestCode )
{
	UInt j;
	UInt uiNumOperationPointsMinus1 = pcViewScalInfoSei->getNumOperationPointsMinus1();
	pcScalableTestCode->WriteUVLC( uiNumOperationPointsMinus1 );
	for( UInt uiOpId = 0; uiOpId <= uiNumOperationPointsMinus1; uiOpId++ )
	{
		pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getOperationPointId( uiOpId ) );
		pcScalableTestCode->WriteCode( pcViewScalInfoSei->getPriorityId( uiOpId ), 5 );
		pcScalableTestCode->WriteCode( pcViewScalInfoSei->getTemporalId( uiOpId ), 3 );

		UInt uiNumTargetOutputViewsMinus1 = pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId );//SEI JJ
		pcScalableTestCode->WriteUVLC( uiNumTargetOutputViewsMinus1 );//SEI JJ

		for( j = 0; j <= uiNumTargetOutputViewsMinus1; j++ )//SEI JJ
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getViewId( uiOpId, j ) );

		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) );
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) );
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) );
		if(!pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId ))//SEI JJ
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) );//SEI JJ 
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) );//SEI JJ 
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag( uiOpId ) );//SEI JJ


		if( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getOpProfileLevelIdc( uiOpId ), 8 );//SEI JJ
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet0Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet1Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet2Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet3Flag( uiOpId ) );
			//bug_fix_chenlulu{
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet4Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet5Flag( uiOpId ) );
			pcScalableTestCode->WriteCode( 0, 2 );
			//pcScalableTestCode->WriteCode( 0, 4 );
			//bug_fix_chenlulu}
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getOpLevelIdc( uiOpId ), 8 );
		}

		if( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getAvgBitrate( uiOpId ), 16 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getMaxBitrate( uiOpId ), 16 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getMaxBitrateCalcWindow( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getConstantFrmRateIdc( uiOpId ), 2 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getAvgFrmRate( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) )//SEI JJ
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ) );//SEI JJ
			for( j = 0; j < pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ); j++ )//SEI JJ
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getDirectlyDependentViewId( uiOpId, j ) );//SEI JJ
		}
		else
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getViewDependencyInfoSrcOpId( uiOpId ) );//SEI JJ

		if( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) )//SEI JJ
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( j = 0; j <= pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getSeqParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
			//{{SEI JJ
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ) );//SEI JJ
			for ( j=0;j<=pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ); j++)//SEI JJ
				pcScalableTestCode->WriteUVLC(pcViewScalInfoSei->getSubsetSeqParameterSetIdDelta(uiOpId,j));//SEI JJ
			//}}SEI JJ		
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ) );//SEI JJ
			for( j = 0; j <= pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ); j++ )//SEI JJ
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getPicParameterSetIdDelta( uiOpId, j ) );//SEI JJ
		}
		else
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getParameterSetsInfoSrcOpId( uiOpId ) );//SEI JJ 
		//{{SEI JJ
		if (pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag(uiOpId))
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMotionVectorsOverPicBoundariesFlag(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxBytesPerPicDenom(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxBitsPerMbDenom(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthHorizontal(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthVertical(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumReorderFrames(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxDecFrameBuffering(uiOpId));
		}
        //}}SEI JJ
	}// for

  return Err::m_nOK;
}

