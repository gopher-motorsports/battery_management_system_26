/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "lookupTable.h"
#include "utils.h"

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

/*!
    @brief   Interpolate a value y given two endpoints (x1, y1) and (x2, y2) and an input value x
    @param   x - The input value to interpolate
    @param   x1 - The x-coordinate of the first endpoint
    @param   x2 - The x-coordinate of the second endpoint
    @param   y1 - The y-coordinate of the first endpoint
    @param   y2 - The y-coordinate of the second endpoint
    @return  The interpolated value of y at x
*/
static float interpolate(float x, float x1, float x2, float y1, float y2)
{
    //if infinite slope, return y2
    if (fequals(x1, x2))
    {
        return y2;
    }
    else
    {
        // map input x to y axis with slope m = (y2-y1)/(x2-x1)
        return (x - x1) * ((y2-y1) / (x2-x1)) + y1;
    }
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */
/*!
    @brief   Look up the value of a function in a lookup table, given an input value
    @param   x - The input value to look up in the table
    @param   table - A pointer to the lookup table containing the function values
    @return  The interpolated value of the function corresponding to the input value, or 0 if an error occurs
*/
float lookup(float x, const LookupTable_S* table)
{
    int16_t index = (int16_t)((x + table->xOffset) / table->xScale);

    if(index < 0)
    {
        index = 0;
    }
    else if(index > (table->size - 2))
    {
        index = (table->size - 2);
    }

    //Interpolate temperature from lookup table
    return interpolate(x, (index * table->xScale), ((index + 1) * table->xScale), table->y[index], table->y[index + 1]);
}
