/* lolly -- load and draw lollys */

/* Copyright (C) 2019 The Regents of the University of California 
 * See README in this or parent directory for licensing information. */

#include "common.h"
#include "obscure.h"
#include "hgTracks.h"
#include "bedCart.h"
#include "bigWarn.h"
#include "lolly.h"
#include "limits.h"
#include "float.h"
#include "bigBedFilter.h"


static int lollyPalette[] =
{
0x1f77b4, 0xff7f0e, 0x2ca02c, 0xd62728, 0x9467bd, 0x8c564b, 0xe377c2, 0x7f7f7f, 0xbcbd22, 0x17becf
};

struct lolly
{
struct lolly *next;
char *name;
double val;
unsigned start;
unsigned end;
unsigned radius;
unsigned height;
Color color;
};

static void lollyDrawItems(struct track *tg, int seqStart, int seqEnd,
        struct hvGfx *hvg, int xOff, int yOff, int width, 
        MgFont *font, Color color, enum trackVisibility vis)
/* Draw a list of lolly structures. */
{
double scale = scaleForWindow(width, seqStart, seqEnd);
struct lolly *popList = tg->items, *pop;
int trackHeight = tg->lollyCart->height;

if (popList == NULL)
    return;

if ( tg->visibility == tvDense)
    {
    for (pop = popList; pop; pop = pop->next)
        {
        int sx = ((pop->start - seqStart) + .5) * scale + xOff; // x coord of center (lower region)
        hvGfxLine(hvg, sx, yOff, sx , yOff+ tl.fontHeight, pop->color);
        }
    return;
    }

for (pop = popList; pop; pop = pop->next)
    {
    int sx = ((pop->start - seqStart) + .5) * scale + xOff; // x coord of center (lower region)
    hvGfxLine(hvg, sx, yOff + trackHeight, sx , yOff+(trackHeight - pop->height), MG_GRAY);
    }
for (pop = popList; pop; pop = pop->next)
    {
    int sx = ((pop->start - seqStart) + .5) * scale + xOff; // x coord of center (lower region)
    hvGfxCircle(hvg, sx, yOff + trackHeight - pop->radius - pop->height, pop->radius, pop->color, TRUE);
    mapBoxHgcOrHgGene(hvg, pop->start, pop->end, sx - pop->radius, yOff + trackHeight - pop->radius - pop->height - pop->radius, 2 * pop->radius,2 * pop->radius,
                       tg->track, pop->name, pop->name, NULL, TRUE, NULL);
    }
}

void lollyLeftLabels(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width, int height,
	boolean withCenterLabels, MgFont *font, Color color,
	enum trackVisibility vis)
{
int fontHeight = tl.fontHeight+1;
int centerLabel = (height/2)-(fontHeight/2);
if ( tg->visibility == tvDense)
    {
    hvGfxText(hvg, xOff, yOff+fontHeight, color, font, tg->shortLabel);
    return;
    }

hvGfxText(hvg, xOff, yOff+centerLabel, color, font, tg->shortLabel);

if (isnan(tg->lollyCart->upperLimit))
    {
    hvGfxTextRight(hvg, xOff, yOff + 2 * 5 , width - 1, fontHeight, color,
        font, "NO DATA");
    return;
    }

char upper[1024];
safef(upper, sizeof(upper), "%g -",   tg->lollyCart->upperLimit);
hvGfxTextRight(hvg, xOff, yOff + 2 * 5 , width - 1, fontHeight, color,
    font, upper);
char lower[1024];
if (tg->lollyCart->lowerLimit < tg->lollyCart->upperLimit)
    {
    safef(lower, sizeof(lower), "%g _", tg->lollyCart->lowerLimit);
    hvGfxTextRight(hvg, xOff, yOff+height-fontHeight - 2 * 5, width - 1, fontHeight,
        color, font, lower);
    }
}


static int lollyHeight(struct track *tg, enum trackVisibility vis)
/* calculate height of all the lollys being displayed */
{
if ( tg->visibility == tvDense)
    return  tl.fontHeight;

return tg->lollyCart->height;
}

double calcVarianceFromSums(double sum, double sumSquares, bits64 n)
/* Calculate variance. */
{   
double var = sumSquares - sum*sum/n;
if (n > 1)
    var /= n-1;
return var;
}   
    
double calcStdFromSums(double sum, double sumSquares, bits64 n)
/* Calculate standard deviation. */
{   
return sqrt(calcVarianceFromSums(sum, sumSquares, n));
}

int cmpHeight(const void *va, const void *vb)
{
const struct lolly *a = *((struct lolly **)va);
const struct lolly *b = *((struct lolly **)vb);
return a->height - b->height;
}

void lollyLoadItems(struct track *tg)
{
struct lm *lm = lmInit(0);
struct bbiFile *bbi =  fetchBbiForTrack(tg);
struct bigBedInterval *bb, *bbList =  bigBedIntervalQuery(bbi, chromName, winStart, winEnd, 0, lm);
char *bedRow[bbi->fieldCount];
char startBuf[16], endBuf[16];
struct lolly *popList = NULL, *pop;
unsigned lollyField = 5;
struct lollyCartOptions *lollyCart = tg->lollyCart;
char *setting = trackDbSetting(tg->tdb, "lollyField");
if (setting != NULL)
    lollyField = atoi(setting);
double minVal = DBL_MAX, maxVal = -DBL_MAX;
double sumData = 0.0, sumSquares = 0.0;
unsigned count = 0;

int trackHeight = tg->lollyCart->height;
struct bigBedFilter *filters = bigBedBuildFilters(cart, bbi, tg->tdb);
                    
for (bb = bbList; bb != NULL; bb = bb->next)
    {
    bigBedIntervalToRow(bb, chromName, startBuf, endBuf, bedRow, ArraySize(bedRow));

    if (!bigBedFilterInterval(bedRow, filters))
        continue;

    double val = atof(bedRow[lollyField - 1]);
    if (!((lollyCart->autoScale == wiggleScaleAuto) ||  ((val >= lollyCart->minY) && (val <= lollyCart->maxY) )))
        continue;

    if (atoi(bedRow[1]) < winStart)
        continue;

    AllocVar(pop);
    slAddHead(&popList, pop);
    pop->val = val;
    pop->start = atoi(bedRow[1]);
    pop->end = atoi(bedRow[2]);
    pop->name = cloneString(bedRow[3]);
    count++;
    sumData += val;
    sumSquares += val * val;
    if (val > maxVal)
        maxVal = val;
    if (val < minVal)
        minVal = val;
    }

if (count == 0)
    lollyCart->upperLimit = lollyCart->lowerLimit = NAN;
else if (lollyCart->autoScale == wiggleScaleAuto)
    {
    lollyCart->upperLimit = maxVal;
    lollyCart->lowerLimit = minVal;
    }

double range = lollyCart->upperLimit - lollyCart->lowerLimit;
int usableHeight = trackHeight - 2 * 5 * 2; 
for(pop = popList; pop; pop = pop->next)
    {
    pop->radius = 5;
    pop->color = MG_RED;
    if (range == 0.0)
        {
        pop->height = usableHeight ;
        pop->color = lollyPalette[0] | 0xff000000;
        }
    else
        {
        pop->height = usableHeight * (pop->val  - lollyCart->lowerLimit) / range + 5 * 2;
        int colorIndex = 8 * (pop->val  - lollyCart->lowerLimit) / range;
        pop->color = lollyPalette[colorIndex] | 0xff000000;
        }
    }

/*
double average = sumData/count;
double stdDev = calcStdFromSums(sumData, sumSquares, count);

for(pop = popList; pop; pop = pop->next)
    {
    if (pop->val > average + stdDev / 5)
        {
        pop->color = MG_RED;
        pop->radius = 8;
        pop->height = 3 * tl.fontHeight;
        }
    else if (pop->val < average - stdDev / 5)
        {
        pop->color = MG_GREEN;
        pop->radius = 3;
        pop->height = 1 * tl.fontHeight;
        }
    else
        {
        pop->color = MG_GRAY;
        pop->radius = 5;
        pop->height = 2 * tl.fontHeight;
        }
        
    }
    */
slSort(&popList, cmpHeight);
tg->items = popList;
}

static struct lollyCartOptions *lollyCartOptionsNew(struct cart *cart, struct trackDb *tdb, 
                                int wordCount, char *words[])
{
struct lollyCartOptions *lollyCart;
AllocVar(lollyCart);

int maxHeightPixels;
int minHeightPixels;
int defaultHeight;  /*  pixels per item */
int settingsDefault;

cartTdbFetchMinMaxPixels(cart, tdb, MIN_HEIGHT_PER, atoi(DEFAULT_HEIGHT_PER), atoi(DEFAULT_HEIGHT_PER),
                                &minHeightPixels, &maxHeightPixels, &settingsDefault, &defaultHeight);
lollyCart->height = defaultHeight;

lollyCart->autoScale = wigFetchAutoScaleWithCart(cart,tdb, tdb->track, NULL);

double tDbMinY;     /*  data range limits from trackDb type line */
double tDbMaxY;     /*  data range limits from trackDb type line */
char *trackWords[8];     /*  to parse the trackDb type line  */
int trackWordCount = 0;  /*  to parse the trackDb type line  */
wigFetchMinMaxYWithCart(cart, tdb, tdb->track, &lollyCart->minY, &lollyCart->maxY, &tDbMinY, &tDbMaxY, trackWordCount, trackWords);
lollyCart->upperLimit = lollyCart->maxY;
lollyCart->lowerLimit = lollyCart->minY;

return lollyCart;
}

void lollyMethods(struct track *track, struct trackDb *tdb, 
                                int wordCount, char *words[])
/* bigLolly track type methods */
{
struct lollyCartOptions *lollyCart = lollyCartOptionsNew(cart, tdb, wordCount, words);
char *ourWords[2];
ourWords[0] = "bigBed";
ourWords[1] = "4";
bigBedMethods(track, tdb,2,ourWords);
if (tdb->visibility == tvDense)
    return;
track->loadItems = lollyLoadItems;
track->drawItems = lollyDrawItems;
track->totalHeight = lollyHeight; 
track->drawLeftLabels = lollyLeftLabels;
track->lollyCart = lollyCart;
}
