kentSrc = ../..
include ../../inc/common.mk

HG_INC += -I../../optimalLeaf
L += ${HALLIBS} ${MYSQLLIBS}
MYLIBDIR = ../../lib/${MACHTYPE}
MYLIBS =  ${MYLIBDIR}/jkhgap.a ${MYLIBDIR}/jkweb.a ${MYLIBDIR}/optimalLeaf.a
COPT = -ggdb

A = hgTracks
RENDERA = hgRenderTracks

COMMONO = adjacencyTrack.o pubsTracks.o altGraphXTrack.o bamTrack.o barChartTrack.o bedGraph.o \
	bedTrack.o  bigBedTrack.o bigWarn.o bigWigTrack.o \
	chainTrack.o cgapSageTrack.o chromGraphTrack.o config.o container.o contigTrack.o \
	coverageTrack.o cytoBandTrack.o cutterTrack.o \
	expRatioTracks.o factorSource.o hgTracks.o interactTrack.o \
	gencodeTracks.o goldTrack.o gtexTracks.o gtexEqtlClusterTrack.o gvfTrack.o \
	lolly.o longRangeTrack.o loweLabTracks.o lrgTrack.o \
	mafTrack.o makeItemsTrack.o menu.o multiWig.o netTrack.o peptideAtlasTrack.o pslTrack.o \
	rmskJoinedTrack.o rmskTrack.o \
	sampleTracks.o searchTracks.o simpleTracks.o snakeTrack.o sortWig.o switchGear.o transMapTracks.o \
	wabaTrack.o wigTrack.o cds.o \
	wigMafTrack.o encode.o variation.o vcfTrack.o transcriptome.o\
	rnaFoldTrack.o retroGene.o hapmapTrack.o rnaPLFoldTrack.o\
	wiki.o wikiTrack.o imageV2.o extTools.o

O = mainMain.o ${COMMONO} 

export RENDERO = renderMain.o ${COMMONO}

include ../../inc/cgi_build_rules.mk

ifeq (${GBROWSE}, 1)
  GBROWSE_D=-DGBROWSE
  A = gbrowseTest
  O = $A.o oneTrack.o \
      bedGraph.o cds.o chainTrack.o mafTrack.o netTrack.o pslTrack.o \
      simpleTracks.o wigMafTrack.o wigTrack.o imageV2.o
else
  GBROWSE_D=
endif

%.o: %.c
	${CC} ${COPT} ${CFLAGS} ${GBROWSE_D} ${LOWELAB_DEFS} ${HG_DEFS} ${HG_WARN} ${HG_INC} ${XINC} -o $@ -c $<

mgsc: strip
	mv $A ${CGI_BIN}/$A

compile: compileMain ${RENDERA}

compileMain: $O
	${CC} ${CFLAGS} -o $A $O ${MYLIBS} ${L}
	cp extTools.ra ${CGI-BIN_DEST}

${RENDERA}: ${RENDERO}
	${MAKE} -f makefile.hgRenderTracks ${MAKECMDGOALS}

wigTrack.o:	wigCommon.h
bedGraph.o:	wigCommon.h

switchGear.o:	switchGear.h
cds.o:		cds.h
hgTracks.o:	cds.h
pslTrack.o:	cds.h

clean::
	rm -f ct_hgwdev_*.bed hgtIdeo_*.gif hgt_*.gif
	rm -f ${RENDERA} ${RENDERO}
