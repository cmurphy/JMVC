#include "MVCScalableModifyCode.h"

MVCScalableModifyCode::MVCScalableModifyCode() :
	m_pcBinData( NULL ),
	m_pulStreamPacket( NULL ),
	m_uiBitCounter( 0 ),
	m_uiPosCounter( 0 ),
	m_uiCoeffCost ( 0 ),
	m_uiDWordsLeft    ( 0 ),
	m_uiBitsWritten   ( 0 ),
	m_iValidBits      ( 0 ),
	m_ulCurrentBits   ( 0 )
{

}

MVCScalableModifyCode::~MVCScalableModifyCode()
{
}


ErrVal
MVCScalableModifyCode::Destroy()
{
	delete this;
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::init( ULong* pulStream )
{
	ROT( pulStream == NULL );
	m_pulStreamPacket = pulStream;

	m_uiDWordsLeft = 0x400/4;
	m_iValidBits = 32;
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteUVLC( UInt uiValue )
{
	UInt uiLength = 1;
	UInt uiTemp = ++uiValue;

	while( uiTemp != 1 )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}

	RNOK( Write( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteCode( UInt uiValue, UInt uiLength )
{
	RNOK( Write( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteFlag( Bool bFlag )
{
	RNOK( Write( bFlag? 1 : 0 , 1) );
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::Write( UInt uiBits, UInt uiNumberOfBits )
{
	m_uiBitsWritten += uiNumberOfBits;

	if( (Int)uiNumberOfBits < m_iValidBits)  // one word
	{
		m_iValidBits -= uiNumberOfBits;

		m_ulCurrentBits |= uiBits << m_iValidBits;

		return Err::m_nOK;
	}


	ROT( 0 == m_uiDWordsLeft );
	m_uiDWordsLeft--;

	UInt uiShift = uiNumberOfBits - m_iValidBits;

	// add the last bits
	m_ulCurrentBits |= uiBits >> uiShift;

	*m_pulStreamPacket++ = xSwap( m_ulCurrentBits );


	// note: there is a problem with left shift with 32
	m_iValidBits = 32 - uiShift;

	m_ulCurrentBits = uiBits << m_iValidBits;

	if( 0 == uiShift )
	{
		m_ulCurrentBits = 0;
	}

	return Err::m_nOK;
}
ErrVal
MVCScalableModifyCode::WritePayloadHeader( enum h264::SEI::MessageType eType, UInt uiSize )
{
	//type
	{
		UInt uiTemp = eType;
		UInt uiByte = 0xFF;	
		while( 0xFF == uiByte )
		{
			uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
			uiTemp -= 0xFF;
			RNOK( WriteCode( uiByte, 8 ) );
		}
	}

	// size
	{
		UInt uiTemp = uiSize;
		UInt uiByte = 0xFF;

		while( 0xFF == uiByte )
		{
			uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
			uiTemp -= 0xFF;
			RNOK( WriteCode( uiByte, 8 ) );
		}
	}
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteAlignZero()
{
	return Write( 0, m_iValidBits & 0x7 );
}

ErrVal
MVCScalableModifyCode::WriteTrailingBits()
{
	RNOK( WriteFlag( 1 ) );
	RNOK( WriteAlignZero() );
	return Err::m_nOK;
}

ErrVal 
MVCScalableModifyCode::flushBuffer()
{
	*m_pulStreamPacket = xSwap( m_ulCurrentBits );

	m_uiBitsWritten = (m_uiBitsWritten+7)/8;

	m_uiBitsWritten *= 8;

	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::ConvertRBSPToPayload( UChar* m_pucBuffer,
																				 UChar pulStreamPacket[],
																			UInt& ruiBytesWritten,
																			UInt  uiHeaderBytes )
{
	UInt uiZeroCount    = 0;
	UInt uiReadOffset   = uiHeaderBytes;
	UInt uiWriteOffset  = uiHeaderBytes;

	//===== NAL unit header =====
	for( UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++ )
	{
		m_pucBuffer[uiIndex] = (UChar)pulStreamPacket[uiIndex];
	}

	//===== NAL unit payload =====
	for( ; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++ )
	{
		if( 2 == uiZeroCount && 0 == ( pulStreamPacket[uiReadOffset] & 0xfc ) )
		{
			uiZeroCount                   = 0;
			m_pucBuffer[uiWriteOffset++]  = 0x03;
		}

		m_pucBuffer[uiWriteOffset] = (UChar)pulStreamPacket[uiReadOffset];

		if( 0 == pulStreamPacket[uiReadOffset] )
		{
			uiZeroCount++;
		}
		else
		{
			uiZeroCount = 0;
		}
	}
	if( ( 0x00 == m_pucBuffer[uiWriteOffset-1] ) && ( 0x00 == m_pucBuffer[uiWriteOffset-2] ) )
	{
		m_pucBuffer[uiWriteOffset++] = 0x03;
	}
	ruiBytesWritten = uiWriteOffset;

	return Err::m_nOK;
}




ErrVal
MVCScalableModifyCode::SEICode( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableModifyCode *pcScalableModifyCode )
{
	UInt uiNumOperationPointsMinus1 = pcViewScalInfoSei->getNumOperationPointsMinus1();
	pcScalableModifyCode->WriteUVLC( uiNumOperationPointsMinus1 );
	for( UInt uiOpId = 0; uiOpId <= uiNumOperationPointsMinus1; uiOpId++ )
	{
		pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getOperationPointId( uiOpId ) );
		pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getPriorityId( uiOpId ), 5 );
		pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getTemporalId( uiOpId ), 3 );

		UInt uiNumTargetOutputViewsMinus1 = pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId );//SEI JJ
		pcScalableModifyCode->WriteUVLC( uiNumTargetOutputViewsMinus1 );//SEI JJ

		for( UInt j = 0; j <= uiNumTargetOutputViewsMinus1; j++ )//SEI JJ
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getViewId( uiOpId, j ) );

		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) );
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) );
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) );
		if(!pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId ))//SEI JJ 
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) );//SEI JJ 
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) );//SEI JJ
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag( uiOpId ) );///SEI JJ

		if( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) )
		{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getOpProfileLevelIdc( uiOpId ), 8 );//SEI JJ
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet0Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet1Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet2Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet3Flag( uiOpId ) );
			//bug_fix_chenlulu{
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet4Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet5Flag( uiOpId ) );
			pcScalableModifyCode->WriteCode( 0, 2 );
			//pcScalableModifyCode->WriteCode( 0, 4 );
			//bug_fix_chenlulu{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getOpLevelIdc( uiOpId ), 8 );
		}

		if( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getAvgBitrate( uiOpId ), 16 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getMaxBitrate( uiOpId ), 16 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getMaxBitrateCalcWindow( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getConstantFrmRateIdc( uiOpId ), 2 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getAvgFrmRate( uiOpId ), 16 );
		}


		if( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) )//SEI JJ
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j < pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ); j++ )//SEI JJ 
				pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getDirectlyDependentViewId( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getViewDependencyInfoSrcOpId( uiOpId ) );//SEI JJ 

		if( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) )//SEI JJ
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getSeqParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ) );//SEI JJ
			for ( UInt j=0;j<=pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ); j++)//SEI JJ
				pcScalableModifyCode->WriteUVLC(pcViewScalInfoSei->getSubsetSeqParameterSetIdDelta(uiOpId,j));//SEI JJ
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getPicParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getParameterSetsInfoSrcOpId( uiOpId ) );//SEI JJ 
		if (pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag(uiOpId))
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMotionVectorsOverPicBoundariesFlag(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMaxBytesPerPicDenom(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMaxBitsPerMbDenom(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthHorizontal(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthVertical(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumReorderFrames(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMaxDecFrameBuffering(uiOpId));
		}

	}// for

	return Err::m_nOK;
}
