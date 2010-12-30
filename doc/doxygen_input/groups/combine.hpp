/*!
\defgroup combine combine: add a geometry to a bounding box
\par Geometries:
- \b box + \b box -> \b box: the box will be combined with the other box
    \image html combine_box_box.png
    \note Also if the input box is incorrect, the box will correctly updated
- \b box + \b point -> \b box: the box will combined with the point
    \image html combine_box_point.png
- \b box + \b segment -> \b box
*/
