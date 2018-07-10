

import random

#-------------------------------------------------------------------------
# Create list of relative coordinates of pixels in neighborhood (center of neighborhood is 0,0)...

def windowCoord(window_r):

    winPosLst = []  # relative coordinate list

    # within neighborhood extent, find and record coordinates of pixels that are within specified
    # radius from center. Coordinates are relative to the center...
    for rowNo in xrange(-window_r,window_r + 1):    
        for colNo in xrange(-window_r,window_r + 1):
            if not (rowNo == 0 and colNo == 0):     # for all pixels except center of neighborhood
                d = (colNo**2 + rowNo**2)**.5       # calculate distance to center
                if d <= window_r:                   # if distance is within specified radius
                    winPosLst.append([rowNo,colNo]) # add coordinates to list

    return winPosLst


#-------------------------------------------------------------------------
# URBANIZED AREA ANALYSIS...

# CLASSIFY LAND COVER INTO THE FOLLOWING CATEGORIES:
#   (1) URBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness > 50%
#   (2) SUBURBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness < 50% and > 10%
#   (3) RURAL ZONE BUILT-UP AREA: built-up pixels with imperviousness < 10%
#   (6) WATER: water

# Imperviousness defined as the % of imperviousness within a radius of 564 meters (circlular
# neighborhood with an area of 1 sq km)

# Requires text files for slope, land cover, and urban landscape. Requires neighborhood radius
def urbAreaClasses(lcTxtFile,urbLandscpe_txt,window_r,studyArea_pntLst):


    #------------------------------------
    # Create list of relative coordinates of pixels in neighborhood (center of neighborhood is 0,0)...
    winPosLst = windowCoord(window_r)

    #------------------------------------
    # Prepare for % imperviousness calculations...

    # open text files...  
    o_lc = file(lcTxtFile)      # input: land cover
    o_urbLandscape = file(urbLandscpe_txt, "w") # output: urban landscape
           
    # get all lines from land cover txt file...
    lines = o_lc.readlines()

    # write header lines to urban landscape txt file...
    o_urbLandscape.writelines(lines[:6])

    # get raster properties...
    ncols = int(lines[0].split(" ")[-1])
    nrows = int(lines[1].split(" ")[-1])
    Xmin = float(lines[2].split(" ")[-1])
    Ymin = float(lines[3].split(" ")[-1])
    cellsize = float(lines[4].split(" ")[-1])

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)

    inStudy_lst = []

    for rowNo in xrange(nrows):
        inStudy_lst.append(bounds(studyArea_pntLst,Y))
##        boundLst = bounds(studyArea_pntLst,Y)
        Y -= cellsize

    # omit header lines from land cover data list...
    lines = lines[6:]

##    # get number of rows and columns for land cover raster (slope raster should have same extent)...
##    ncols = len(lines[0].split(" ")[:-1])
##    nrows = len(lines)

    # set initial min and max row number for neighborhood...
    minRow = -window_r
    maxRow = window_r

    wRowDct = {}    # dictionary for neighborhood pixel values

    # populate dictionary with neighborhood pixel values...
    for wRowNo in xrange(0, maxRow+1):
        row = lines[wRowNo].split(" ")[:-1]     # extract row pixel values
        wRowDct[wRowNo] = row       # add row pixel values to dictionary
     
    #------------------------------------
    # CALCULATE % IMPERVIOUSNESS FOR EACH LAND COVER PIXEL (EXCEPT WATER)

    rowCnt = sumPerUrb = numPix = edgeCnt = 0

    # for each row in image...
    for rowNo in xrange(nrows):


        X = Xmin - cellsize / 2

        inRow_lst = inStudy_lst[rowNo]

        urbLC_line = ""     # txt file line for urban landscape image
        
        # for each column in row...                                             
        for colNo in xrange(ncols):
    
            X += cellsize
           
            centerPix = wRowDct[rowNo][colNo]   # value for center pixel

            #----------------------------
            # if center pixel is NoData ("0") or water ("2")
            # skip calculation. Update urban landscape txt file line...
                           
            if centerPix not in ['1','2','3']:    # NoData 
                urbLC_line += "0 "
                continue
                            
            if centerPix == "2":    # water
                urbLC_line += "7 "
                continue
            

            #----------------------------
            # if center pixel is urban or other non-urban, calculate imperviousness...

            urbCnt = pixCnt = edges = 0     # count of neighborhood urban pixels and neighborhood total
                                    # pixels (excluding NoData, water, excessive slope)

            # for each pixel in neighborhood...
            for x,y in winPosLst:   # get relative coordinates

                y += colNo
                if y < 0 or y > ncols - 1:
                    continue

                x += rowNo
                if x < 0 or x > nrows-1:
                    continue

                # try to get pixel value...
               
                # get pixel value from neighborhood data dictionary...
                pix = wRowDct[x][y]

                #--------------------
                # if pixel value is urban, cnt as urban
                if pix == "3":      # urban pixel
                    urbCnt += 1

                # if other non-urban, count pixel
                elif pix == "1" or pix == "2":    # other non-urban pixel
                    pixCnt += 1

               
            #----------------------
            # ADJACENCY INDEX...
            
            # test for adjacency with non-urban pixels...
            for x,y in [[-1,0],[1,0],[0,1],[0,-1]]:

                y += colNo
                if y < 0 or y > ncols - 1:
                    continue

                x += rowNo
                if x < 0 or x > nrows-1:
                    continue
                
                # get pixel value from neighborhood data dictionary...
                pix = wRowDct[x][y]

                #--------------------
                # if pixel value is urban, cnt as urban
                if pix != "3":      # urban pixel
                    edges += 1
                    break                    
            #----------------------
        
            # calculate % imperviousness for pixel...
            if pixCnt + urbCnt == 0:
                urbLC_line += "0 "
                continue
            
            perUrb = float(urbCnt) / (urbCnt + pixCnt)

            #----------------------------
            # Classify % imperviousness as urban (>0.5), suburban (<0.5 and > 0.1), or rural (<0.1)...

            if centerPix == "3":
                if perUrb > .5:
                    urbLC_line += "1 "  # urban
                elif .1 < perUrb <= .5:
                    urbLC_line += "2 "  # suburban
                else:
                    urbLC_line += "3 "  # rural
            elif centerPix == "1":
                if perUrb > .5:
                    urbLC_line += "4 "  #  urbanized OS
                else:
                    urbLC_line += "6 "  # rural OS
            

            #---------------------------------------
            # TEST IF POINT IS IN STUDY AREA

            if centerPix == "1":
                
                inPnt = 0

                for x1,x2 in inRow_lst:
                    if x1 <= X <= x2:
                        inPnt = 1
                        break

                if inPnt == 0:
                    continue

                sumPerUrb += perUrb
                numPix += 1

                if edges > 0:
                    edgeCnt += 1

                   
        urbLC_line += "\n"

        # update urban landscape text file...
        o_urbLandscape.write(urbLC_line)

        #--------------------------------   
        # Update neighborhood data dictionary - move down one row...

        # if min row is within image extent...
        if minRow > 0:
            del wRowDct[minRow] # delete top row

        # update min / max row...
        minRow += 1
        maxRow += 1

        # if max row is within extent...
        if maxRow < nrows:
            # add new max row to dictionary...
            wRowDct[maxRow] = lines[maxRow].split(" ")[:-1]

    # close text files... 
    o_lc.close()
    o_urbLandscape.close()

    

    openness = 1-(sumPerUrb / numPix)
    edge_index = float(edgeCnt) / numPix

    

##    return edge_index, openness, inStudy_lst
    return edge_index, openness, inStudy_lst
    


#-------------------------------------------------------------------------
# URBAN FOOTPRINT ANALYSIS...

# CLASSIFY LAND COVER INTO THE FOLLOWING CATEGORIES:
#   (1) URBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness > 50%
#   (2) SUBURBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness < 50% and > 10%
#   (3) RURAL ZONE BUILT-UP AREA: built-up pixels with imperviousness < 10%
#   (6) WATER: water

# Imperviousness defined as the % of imperviousness within a radius of 564 meters (circlular
# neighborhood with an area of 1 sq km)

# Requires text files for slope, land cover, and urban landscape. Requires neighborhood radius
def urbFtptClasses(lcTxtFile,urbLandscpe_txt,window_r,studyArea_pntLst):


    #------------------------------------
    # Create list of relative coordinates of pixels in neighborhood (center of neighborhood is 0,0)...
    winPosLst = windowCoord(window_r)

    #------------------------------------
    # Prepare for % imperviousness calculations...

    # open text files...  
    o_lc = file(lcTxtFile)      # input: land cover
    o_urbLandscape = file(urbLandscpe_txt, "w") # output: urban landscape
            
    # get all lines from land cover txt file...
    lines = o_lc.readlines()

    # write header lines to urban landscape txt file...
    o_urbLandscape.writelines(lines[:6])

    # get raster properties...
    ncols = int(lines[0].split(" ")[-1])
    nrows = int(lines[1].split(" ")[-1])
    Xmin = float(lines[2].split(" ")[-1])
    Ymin = float(lines[3].split(" ")[-1])
    cellsize = float(lines[4].split(" ")[-1])

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)

    inStudy_lst = []

    for rowNo in xrange(nrows):
        inStudy_lst.append(bounds(studyArea_pntLst,Y))
##        boundLst = bounds(studyArea_pntLst,Y)
        Y -= cellsize

    # omit header lines from land cover data list...
    lines = lines[6:]

##    # get number of rows and columns for land cover raster (slope raster should have same extent)...
##    ncols = len(lines[0].split(" ")[:-1])
##    nrows = len(lines)

    # set initial min and max row number for neighborhood...
    minRow = -window_r
    maxRow = window_r

    wRowDct = {}    # dictionary for neighborhood pixel values

    # populate dictionary with neighborhood pixel values...
    for wRowNo in xrange(0, maxRow+1):
        row = lines[wRowNo].split(" ")[:-1]     # extract row pixel values
        wRowDct[wRowNo] = row       # add row pixel values to dictionary
     
    #------------------------------------
    # CALCULATE % IMPERVIOUSNESS FOR EACH LAND COVER PIXEL (EXCEPT WATER)

    rowCnt = sumPerUrb = numPix = edgeCnt = 0

    # for each row in image...
    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2

        inRow_lst = inStudy_lst[rowNo]

        urbLC_line = ""     # txt file line for urban landscape image
        
        # for each column in row...                                             
        for colNo in xrange(ncols):

            X += cellsize
           
            centerPix = wRowDct[rowNo][colNo]   # value for center pixel

            #----------------------------
            # if center pixel is NoData ("0") or water ("2")
            # skip calculation. Update urban landscape txt file line...
                      
            if centerPix == "1":    # non-water, nonurban
                urbLC_line += "4 "
                continue
            if centerPix == "2":    # water
                urbLC_line += "7 "
                continue
            if centerPix not in ['1','2','3']:    # NoData 
                urbLC_line += "0 "
                continue
            

            #----------------------------
            # if center pixel is urban or other non-urban, calculate imperviousness...

            urbCnt = pixCnt = edges = 0     # count of neighborhood urban pixels and neighborhood total
                                    # pixels (excluding NoData, water, excessive slope)

            # for each pixel in neighborhood...
            for x,y in winPosLst:   # get relative coordinates

                y += colNo
                if y < 0 or y > ncols - 1:
                    continue

                x += rowNo
                if x < 0 or x > nrows-1:
                    continue

                # try to get pixel value...
               
                # get pixel value from neighborhood data dictionary...
                pix = wRowDct[x][y]

                #--------------------
                # if pixel value is urban, cnt as urban
                if pix == "3":      # urban pixel
                    urbCnt += 1

                # if other non-urban, count pixel
                elif pix in ['1','2']:    # other non-urban pixel
                    pixCnt += 1

               

            # test for adjacency with non-urban pixels...
##            for x,y in [[-1,0],[1,0],[0,1],[0,-1]]:
##
##                y += colNo
##                if y < 0 or y > ncols - 1:
##                    continue
##
##                x += rowNo
##                if x < 0 or x > nrows-1:
##                    continue
##                
##                # get pixel value from neighborhood data dictionary...
##                pix = wRowDct[x][y]
##
##                #--------------------
##                # if pixel value is urban, cnt as urban
##                if pix != "3":      # urban pixel
##                    edges += 1
##                    break
                    
              
        
            # calculate % imperviousness for pixel...
            if pixCnt + urbCnt == 0:
                urbLC_line += "0 "
                continue
            
            perUrb = float(urbCnt) / (urbCnt + pixCnt)
            
                

            #----------------------------
            # Classify % imperviousness as urban (>0.5), suburban (<0.5 and > 0.1), or rural (<0.1)...

            
            if perUrb > .5:
                urbLC_line += "1 "  #  urban
            elif .1 < perUrb <= .5:
                urbLC_line += "2 "  # suburban
            else:
                urbLC_line += "3 "  # rural

##            #---------------------------------------
##            # TEST IF POINT IS IN STUDY AREA
##
##            inPnt = 0
##
##            for x1,x2 in inRow_lst:
##                if x1 <= X <= x2:
##                    inPnt = 1
##                    break
##
##            if inPnt == 0:
##                continue
##
##            sumPerUrb += perUrb
##            numPix += 1
##
##            if edges > 0:
##                edgeCnt += 1

                   
        urbLC_line += "\n"

        # update urban landscape text file...
        o_urbLandscape.write(urbLC_line)

        #--------------------------------   
        # Update neighborhood data dictionary - move down one row...

        # if min row is within image extent...
        if minRow > 0:
            del wRowDct[minRow] # delete top row

        # update min / max row...
        minRow += 1
        maxRow += 1

        # if max row is within extent...
        if maxRow < nrows:
            # add new max row to dictionary...
            wRowDct[maxRow] = lines[maxRow].split(" ")[:-1]

    # close text files... 
    o_lc.close()
    o_urbLandscape.close()

##    print sumPerUrb
##    print numPix
##    print edgeCnt

##    openness = 1-(sumPerUrb / numPix)
##    edge_index = float(edgeCnt) / numPix


    return inStudy_lst

    


#-------------------------------------------------------------------------
# URBAN FOOTPRINT ANALYSIS...

# CLASSIFY URBANIZED OPEN AREA IN URBAN LANDSCAPE MAP...
# URBANIZED OPEN AREA IS A 100 METER BUFFER AROUND THE URBAN AND
# SUBURBAN BUILT UP AREA

#   (4) URBANIZED OPEN AREA: non-urban pixels < 100 meters from urban and suburban pixels
#   (6) RURAL OPEN AREA: non-urban pixels > 100 meters from urban and suburban pixels


def urbOpenArea(urbClasses_txt,urbLndscp_txt,window_r):


    #------------------------------------
    # Create list of relative coordinates of pixels in neighborhood (center of neighborhood is 0,0)...

    winPosLst = windowCoord(window_r)
    #------------------------------------
       
    # open text files...          
    o_urbClasses = file(urbClasses_txt, "r") # input: urban landscape
    o_urbLndscp = file(urbLndscp_txt, "w")    # output: core / edge open area
         
    # get all lines from land cover txt file...
    lines = o_urbClasses.readlines()

    # write header lines to urban landscape txt file...
    o_urbLndscp.writelines(lines[:6])

##        # get raster NoData value...
##        noData = lines[5].split(" ")[-1].rstrip("\n")

    # omit header lines from urban landscape data list...
    lines = lines[6:]

    # get number of rows and columns for raster...
    ncols = len(lines[0].split(" ")[:-1])
    nrows = len(lines)

    # set initial min and max row number for neighborhood...
    minRow = -window_r
    maxRow = window_r

    wRowDct = {}    # dictionary for neighborhood pixel values

    # populate dictionary with neighborhood pixel values...
    for wRowNo in xrange(0, maxRow+1):
        row = lines[wRowNo].split(" ")[:-1]     # extract row pixel values
        wRowDct[wRowNo] = row                   # add row pixel values to dictionary
     
    #------------------------------------
    # DETERMINE EDGE OPEN AREA (100 meter buffer around built-up)...

    rowCnt = 0
##        edgeCnt = totalPix = 0

    # for each row in image...
    for rowNo in xrange(nrows):

        urbLnd_line = ""     # txt file line for urban landscape image
        
        # for each column in row...                                             
        for colNo in xrange(ncols):

            centerPix = wRowDct[rowNo][colNo]   # value for center pixel

            if centerPix != "4":  # if built-up or water
                urbLnd_line += "%s " % centerPix 
                continue                   

            
            centerPix = "6"     # default to rural OS
##                totalPix += 1

            #-----------------------------------------------
            # Determine if built-up pixel is within window. If so, current pixel is edge,
            # otherwise it is core open...
              
            # for each pixel in neighborhood...
            for x,y in winPosLst:   # get relative coordinates

                # try to get pixel value...
                try:
                    # get pixel value from neighborhood data dictionary...
                    pix = wRowDct[rowNo+x][colNo+y]

                    #--------------------
                    # if pixel value is urban or suburban, center pixel is urbanized open
                    if pix in ["1","2"]:    # urban pixel
                        centerPix = "4"
                        break    # edge pixel                       

                # errors result from pixel being outside raster extent, skip these pixels
                except:
                    continue
        
            urbLnd_line += "%s " % centerPix       

        # update edge / core text file...
        o_urbLndscp.write("%s\n" % urbLnd_line)

        
        #--------------------------------   
        # Update neighborhood data dictionary - move down one row...

        # if min row is within image extent...
        if minRow > 0:
            del wRowDct[minRow] # delete top row

        # update min / max row...
        minRow += 1
        maxRow += 1

        # if max row is within extent...
        if maxRow < nrows:
            # add new max row to dictionary...
            wRowDct[maxRow] = lines[maxRow].split(" ")[:-1]

    # close text files... 
    o_urbClasses.close()
    o_urbLndscp.close()



#--------------------------------------------------------------------------------------------
# ADD ISOLATED OPEN PATCHES TO LANDSCAPE MAP...


def isolated_open(gapTxtFile, urb_edgeOpen_txt, urbLndscp_txt):

    # open text files...
    o_urb_edgeOpen_txt = file(urb_edgeOpen_txt, 'r') # raster containing urban classes and edge open space
    o_gap = file(gapTxtFile, 'r')      # input: land cover
    o_urbLandscape = file(urbLndscp_txt, "w") # output: urban landscape
            
    for rowNo in range(6):
        line = o_gap.readline()
        o_urbLandscape.write(line)
                
    # get raster properties...
    ncols = int(o_urb_edgeOpen_txt.readline().split(" ")[-1])
    nrows = int(o_urb_edgeOpen_txt.readline().split(" ")[-1])
    Xmin = float(o_urb_edgeOpen_txt.readline().split(" ")[-1])
    Ymin = float(o_urb_edgeOpen_txt.readline().split(" ")[-1])
    cellsize = float(o_urb_edgeOpen_txt.readline().split(" ")[-1])

    o_urb_edgeOpen_txt.readline()

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)
    
    # for each row in image...
    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2

        org_line = o_urb_edgeOpen_txt.readline().split(" ")[:-1]
        gap_line = o_gap.readline().split(" ")[:-1]

        urbLndscp_line = ""
                  
        # for each column in row...                                             
        for colNo in xrange(ncols):

            X+= cellsize

            org_value = org_line[colNo]   # value for current pixel
            gap_value = gap_line[colNo]

            if gap_value == '0' and org_value == '6':   # isolated patch present and current value is rural OS
                urbLndscp_line += "5 "      # new value is isolated open
            else:
                urbLndscp_line += '%s ' % org_value     # value stays the same

        
        # update edge / core text file...
        o_urbLandscape.write("%s\n" % urbLndscp_line)           


    o_urb_edgeOpen_txt.close() # raster containing urban classes and edge open space
    o_gap.close()      # input: land cover
    o_urbLandscape.close()





#--------------------------------------------------------------------------------------------
# GET LIST OF POINTS IN URBANIZED AREA. CALCULATE AREAS OF URBAN LANDSCAPE CLASSES

def Extract_to_StudyArea(urbLndscp_txt, inStudy_lst):


    urbLndscp_clp_txt = "%s_clp.txt" % urbLndscp_txt[:-4]

    o_urbLndscp_txt = file(urbLndscp_txt,"r")

    o_urbClp = file(urbLndscp_clp_txt,"w")
    
    

    for rowNo in range(6):
        line = o_urbLndscp_txt.readline()
        o_urbClp.write(line)
        
    o_urbLndscp_txt.seek(0)  
    
    # get raster properties...
    ncols = int(o_urbLndscp_txt.readline().split(" ")[-1])
    nrows = int(o_urbLndscp_txt.readline().split(" ")[-1])
    Xmin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    Ymin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    cellsize = float(o_urbLndscp_txt.readline().split(" ")[-1])

    o_urbLndscp_txt.readline()

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)
    
    # for each row in image...
    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2

        inRow_lst = inStudy_lst[rowNo]

        urb_line = o_urbLndscp_txt.readline().split(" ")[:-1]

        urbClp_line = ""
              
        # for each column in row...                                             
        for colNo in xrange(ncols):

            X+= cellsize
            
            urbPix = urb_line[colNo]   # value for center pixel

            #---------------------------------------
            # TEST IF POINT IS IN STUDY AREA

            inPnt = 0

            for x1,x2 in inRow_lst:
                if x1 <= X <= x2:
                    inPnt = 1
                    break

            if inPnt == 0:
                urbClp_line += "0 " 
                continue
            
            #---------------------------------------
            urbClp_line += "%s " % urbPix
                    
            
        o_urbClp.write("%s\n" % urbClp_line)
        
        Y -= cellsize

    o_urbLndscp_txt.close()
    o_urbClp.close()

    
    return urbLndscp_clp_txt
                


#------------------------------------------------------
# CALCULATE AREA OF URBAN LANDSCAPE CLASSES...

def classAreas(urbLndscp_clp_txt):
    o_urbLndscp_txt = file(urbLndscp_clp_txt, 'r')

    ncols = int(o_urbLndscp_txt.readline().split(" ")[-1])
    nrows = int(o_urbLndscp_txt.readline().split(" ")[-1])
    

    for rowNo in range(4):
        o_urbLndscp_txt.readline()

    urban_BA = suburban_BA = rural_BA = urban_OA = isolated_OA = rural_OA = 0

    for rowNo in xrange(nrows):
        line = o_urbLndscp_txt.readline()

        urban_BA += line.count('1')
        suburban_BA += line.count('2')
        rural_BA += line.count('3')
        urban_OA += line.count('4')
        isolated_OA += line.count('5')
        rural_OA += line.count('6')


    o_urbLndscp_txt.close()

    return urban_BA, suburban_BA, rural_BA, urban_OA, isolated_OA, rural_OA

#------------------------------------------------------


















## SPRAWL METRICS ALGORITHMS...

#------------------------------------------------------
# Get coordinates of points in the urbanized area...

def urbAreaLst(urbLndscp_clp_txt):
    
    o_urbLndscp_txt = file(urbLndscp_clp_txt, 'r')

    ncols = int(o_urbLndscp_txt.readline().split(" ")[-1])
    nrows = int(o_urbLndscp_txt.readline().split(" ")[-1])
    Xmin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    Ymin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    cellsize = float(o_urbLndscp_txt.readline().split(" ")[-1])

##    pntList = []
   
    o_urbLndscp_txt.readline()


    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)
    urbArea = 0
    sumX = sumY = 0

    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2
        
        line = o_urbLndscp_txt.readline().split(" ")[:-1]

        for colNo in xrange(ncols):
            X+= cellsize
            pix = line[colNo]

            if pix in ['1','2','4','5']:
##                pntList.append([X,Y])
                urbArea += 1
                sumX += X
                sumY += Y
                
        Y-= cellsize

    o_urbLndscp_txt.close()

    avgX = sumX / urbArea
    avgY = sumY / urbArea

##    return pntList, urbArea, avgX, avgY
    return urbArea, avgX, avgY

#------------------------------------------------------
# Get coordinates of proximate center...

def Center(pntList,bndCoord):

    #------------------------------
    # Determine number of iterations needed...
   
    # calculate range of X and Y values...
    difX = bndCoord[2] - bndCoord[0]
    difY = bndCoord[3] - bndCoord[1]

    if abs(difX) > abs(difY):
        d = abs(difX)
    else:
        d = abs(difY)

    # determine number of iterations needed...
    numRuns = 1
    while d > 30:    # minimum precision is within 2 map units
        d /= 2
        numRuns += 1

    #------------------------------
    

    Xmin, Ymin = bndCoord[0], bndCoord[1]
    Xmax, Ymax = bndCoord[2], bndCoord[3]


    for run in range(numRuns):


        testLst = [[Xmin, Ymin],[Xmin, Ymax],[Xmax, Ymax],[Xmax, Ymin]]

        
        minSumD = "0"
        
        for Xt,Yt in testLst:      

##            for Xt,Yt in [[xt-30,yt-30],[xt-30,yt+30],[xt+30,yt+30],[xt+30,yt-30]]:
          
            sumD = 0
           
            for x,y in pntList:
##                    dst = ((Xt-x)**2+(Yt-y)**2)**.5
                dst = (((Xt-x)**2+(Yt-y)**2)**.5)**.5   # square root of distance
                sumD += dst

            if sumD < minSumD:
                minSumD = sumD
                Xb,Yb = Xt,Yt

        Xc = (Xmin + Xmax)/2
        Yc = (Ymin + Ymax)/2

                   
        if Xb < Xc:
            Xmin, Xmax = Xb, Xc
        else:
            Xmin, Xmax = Xc, Xb

        if Yb < Yc:
            Ymin, Ymax = Yb, Yc
        else:
            Ymin, Ymax = Yc, Yb

    return Xc,Yc     

#------------------------------------------------------
# CENTROID OF URBANIZED AREA...

def centroid(urbLndscp_clp_txt):

    o_urbLndscp_txt = file(urbLndscp_clp_txt, 'r')

    ncols = int(o_urbLndscp_txt.readline().split(" ")[-1])
    nrows = int(o_urbLndscp_txt.readline().split(" ")[-1])
    Xmin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    Ymin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    cellsize = float(o_urbLndscp_txt.readline().split(" ")[-1])
   
    o_urbLndscp_txt.readline()

    sumX = sumY = cnt = 0

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)

    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2
        
        line = o_urbLndscp_txt.readline().split(" ")[:-1]

        for colNo in xrange(ncols):
            X+= cellsize
            pix = line[colNo]

            if pix in ['1','2','4','5']:

                sumX += X
                sumY += Y
                cnt += 1

        Y -= cellsize

    # centroid X and Y
    Xc = sumX / cnt
    Yc = sumY / cnt

    o_urbLndscp_txt.close()

    return Xc,Yc


#------------------------------------
# PROXIMITY INDEX...

def proximity(urbLndscp_clp_txt,lc, slp_clp_txt,center_XY,centroid,r):

    Xc, Yc = center_XY[0], center_XY[1]
    Xcentroid, Ycentroid = centroid[0], centroid[1]

    o_urbLndscp_txt = file(urbLndscp_clp_txt, 'r')
    o_slp_clp_txt = file(slp_clp_txt, 'r')
    o_lc = file(lc,'r')

    ncols = int(o_urbLndscp_txt.readline().split(" ")[-1])
    nrows = int(o_urbLndscp_txt.readline().split(" ")[-1])
    Xmin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    Ymin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    cellsize = float(o_urbLndscp_txt.readline().split(" ")[-1])
   
    o_urbLndscp_txt.readline()

    for line in range(6):
        o_slp_clp_txt.readline()
        o_lc.readline()
        

    sumD = sumD_sqr = cnt = in_EAC = 0
    nonUA_dLst = []
    UA_dLst = []

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)

    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2
        
        line = o_urbLndscp_txt.readline().split(" ")[:-1]
        slp_line = o_slp_clp_txt.readline().split(" ")[:-1]
        lc_line = o_lc.readline().split(" ")[:-1]

        for colNo in xrange(ncols):
            X+= cellsize

            d = ((X-Xc)**2 + (Y-Yc)**2)**.5
            d_centroid = ((X-Xcentroid)**2 + (Y-Ycentroid)**2)**.5
        
            pix = line[colNo]
            slpPix = slp_line[colNo]
            lcPix = lc_line[colNo]

            if pix in ['1','2','4','5']:
                if slpPix == '0':
                    UA_dLst.append(d)
                
                sumD += d
                sumD_sqr += d_centroid**2
                cnt += 1

                if d <= r:
                    in_EAC += 1

            elif lcPix in ['1','3']:
                if slpPix == '0':
                    nonUA_dLst.append(d)

        Y -= cellsize

    D_to_Center = sumD / cnt
    D_to_Center_sqr = sumD_sqr / cnt

    

    # NET EXCHANGE INDEX...

    nonUA_dLst += UA_dLst

    nonUA_dLst.sort()
    area_pix = 0

    for d in nonUA_dLst:
        area_pix += 1
        if area_pix >= cnt: break

    nEAC_r = d
    print nEAC_r

    in_nEAC = 0
    
    for d in UA_dLst:
        if d <= nEAC_r:
            in_nEAC += 1            
        
    o_urbLndscp_txt.close()
    o_slp_clp_txt.close()
    o_lc.close()
    

    return D_to_Center, D_to_Center_sqr, in_EAC, in_nEAC


#-------------------------------------------------------------------
# COHESION INDEX...


def interpointDistance(urbLndscp_clp_txt): # requires list of XY coordinates of points in shape


    # get list of point coordinates in urbanized area...
    ptList = urbAreaPnts(urbLndscp_clp_txt)


##    ptList = urbAreaPntLst

    # number of points in shape...
    numPts = len(ptList)

    samplSize = 1000    # number of points in sample
    samples = 30        # number of samples
        
    avgD = avgD_sqr = 0    # average interpoint distance 

    # run specified number of samples...
    for t in range(samples):

        sumD = sumDsqr = 0     # cumulative distance
        cnt = 0         # number of calculations
        
        # select a random sample of shape points...
        sampLst = random.sample(ptList, samplSize)

        # for each point in sample, calculate distance between it and
        # every other point...
        for pt1 in sampLst:
            
            # get coordinates of a point...   
            X1 = pt1[0]
            Y1 = pt1[1]

            # calculate distance to all other points in sample...
            for pt2 in sampLst:

                # skip pt2 if it is the same as pt1...
                if pt1 == pt2: continue 

                # get coord. of point from sample...
                X2 = pt2[0]
                Y2 = pt2[1]

                # calculate distance to pt1...
                d = ((X1-X2)**2 + (Y1-Y2)**2)**.5
                
                # cumulative interpoint distance...
                sumD += d
                sumDsqr += d**2

                # number of calculations...
                cnt += 1
                    
        # average interpoint distance...
        avgD += (sumD / cnt) / samples
        avgD_sqr += (sumDsqr / cnt) / samples
    
    return avgD, avgD_sqr



#------------------------------------------------
# XY LIST OF POINTS IN URBANIZED AREA...

def urbAreaPnts(urbLndscp_clp_txt):

    o_urbLndscp_txt = file(urbLndscp_clp_txt, 'r')

    ncols = int(o_urbLndscp_txt.readline().split(" ")[-1])
    nrows = int(o_urbLndscp_txt.readline().split(" ")[-1])
    Xmin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    Ymin = float(o_urbLndscp_txt.readline().split(" ")[-1])
    cellsize = float(o_urbLndscp_txt.readline().split(" ")[-1])
   
    o_urbLndscp_txt.readline()

    urbAreaPnts_lst = []

    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)

    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2
        
        line = o_urbLndscp_txt.readline().split(" ")[:-1]

        for colNo in xrange(ncols):
            X+= cellsize
            pix = line[colNo]

            if pix in ['1','2','4','5']:

                urbAreaPnts_lst.append([X,Y])

        Y -= cellsize

    
    o_urbLndscp_txt.close()

    return urbAreaPnts_lst








#-----------------------------------------------------------------
#-----------------------------------------------------------------




##def center(PntLst):
##    
##    #------------------
##    # Get centroid of urban built-up area (urban + suburban BA)...
##
##    sumX = sumY = 0
##    for pt in PntLst:
##        X, Y = pt[0], pt[1]
##        sumX += X
##        sumY += Y
##
##    numPts = len(PntLst)
##
##    # centroid X and Y
##    Xc = sumX / numPts
##    Yc = sumY / numPts
##    #------------------
##
##    return Xc,Yc
##
##
###-------------------------------------------------------------------
### PROXIMITY INDEX...
##
##def proximity(PntLst,Xc,Yc):
##
##    sumD = 0
##
##    for pt in PntLst:
##        X, Y = pt[0], pt[1]
##        sumD += ((X-Xc)**2 + (Y-Yc)**2)**.5
##
##    numPts = len(PntLst)
##
##    D_to_Center = sumD / numPts
##
##    return D_to_Center










#-------------------------------------------------------------------------------
# EXCHANGE INDEX

def exchange(PntLst,Xc,Yc,r):

    numPts = len(PntLst)
    
    inPix = 0   # counter for shape pixels in land constrained circle

    # for each pixel in shape points list...
    for x,y in PntLst:

        # distance to proximate center...
        d = ((x - Xc)**2 + (y - Yc)**2)**.5

        # if distance is less than land constrained circle radius...
        if d <= r:
            inPix += 1  # count pixel

    # area exchange index...
    ExchangeIndex = inPix / float(numPts)

    return ExchangeIndex
    
    
#-------------------------------------------------------------------------------
# SPIN INDEX...

def spin(PntLst,Xc,Yc):
   
    sum_dsqr = 0    # sum of distance squared...

    cnt = 0    # count of number of pixels...

    # for each shape point...
    for X,Y in PntLst:

        # distance to center squared...
        dsqr = (X-Xc)**2+(Y-Yc)**2
        
        # sum of squared distances...
        sum_dsqr += dsqr

        # count of points...
        cnt += 1

    return sum_dsqr / cnt



#--------------------------------------------------------------------------------------------
# TEST IF POINT IS IN POLYGON (HELPER ALGORITHM)

# To test if a point is in a shape, a vertical line is drawn from the point to
# the lowest extent. The number of intersections between the vertical line and
# the shape perimeter is determined. If the number of intersections is more than
# zero and is odd, then the test point is in the shape.


# requires list of feature XY coordinates (feature array containing part arrays)
# requires coordinates for a test point
def pntInShp(featLst,testPnt):

    # XY coordinates of the test point...   
    Xr, Yr = testPnt[0], testPnt[1]

    intersect = 0   # number of intersections
    inPoly = 0

    for partPnts in featLst:

        # for each point in feature part...
        for i in xrange(len(partPnts)-1):

            # get X coordinates of segment end points...
            X1,X2 = partPnts[i][0], partPnts[i+1][0]

            if Xr < X1 and Xr < X2: continue
            if Xr > X1 and Xr > X2: continue
         

            # get Y coordinates of segment end points...
            Y1,Y2 = partPnts[i][1], partPnts[i+1][1]

            if Yr < Y1 and Yr < Y2:
                continue

            # if X values are the same, skip...
            if X1 == X2: continue

            if Y1 == Y2:
                intersect += 1
                continue

            # slope of line between pt1 and pt2...
            m = (Y2-Y1) / (X2-X1)

            # y value on line that has X value equal to test point X coordinate...
            y = m*(Xr-X1)+Y1


            # if test point Y is greater than y on the line between pt1 and pt2...
            if Yr >= y:
                # then vertical line containing test point intersects polygon boundary...
                intersect += 1

        # if more than 1 intersection and total number is odd,
        if intersect > 0 and intersect % 2 != 0:
                # then test point is inside polygon...
                inPoly = 1
                break
    

    return inPoly   # 1 if test point is in shape, 0 if not in shape


#------------------------------------------------------------------
# IDENTIFY GRID CELLS IN ROW THAT BOUND ROW PIXELS IN STUDYAREA...

def bounds(studyArea_pntLst,Y):

    intersectLst = []

    studyArea_pntLst = studyArea_pntLst[0]

    for pos in range(len(studyArea_pntLst)-1):

        X1, Y1 = studyArea_pntLst[pos][0], studyArea_pntLst[pos][1]
        X2, Y2 = studyArea_pntLst[pos+1][0], studyArea_pntLst[pos+1][1]      

        #---------------------------------------
        # equation for 1st line...

        # slope of line segment...
        if X1 == X2:
            x = X1  # vertical line
        else:
            m = (Y2-Y1)/(X2-X1)      # calculate slope

            if m == 0:
                continue

            # intercept of segment
            B1 = Y1-(m*X1)

            x = (Y - B1) / m

        # skip if no intersection...
        if not(X1 <= x <= X2 or X2 <= x <= X1): continue
        
        intersectLst.append(x)
        
    intersectLst.sort()

    rangeLst = []

    for pos in range(len(intersectLst)-1):
        X1, X2 = intersectLst[pos], intersectLst[pos+1]

        Xm = X1 + (X2-X1)/2
        
        if pntInShp([studyArea_pntLst],[Xm,Y]) == 1:
            rangeLst.append([X1,X2])

    return rangeLst


#---------------------------------------------------------
# NEW DEVELOPMENT CLASSIFICATION...

def NewDevelopment(urbArea_t1,urbArea_t2,urbFtpt_t1, tempWksp, newDev_txt,gp):

    infill_txt = "%s\\infill.txt" % tempWksp
    otherNewDev_txt = "%s\\otherNewDev.txt" % tempWksp

    o_urbArea_t1_txt = file(urbArea_t1, 'r')
    o_urbArea_t2_txt = file(urbArea_t2, 'r')
    o_inFill_txt = file(infill_txt, 'w')
    o_otherNewDev_txt = file(otherNewDev_txt, 'w')
    

    ncols = int(o_urbArea_t1_txt.readline().split(" ")[-1])
    nrows = int(o_urbArea_t1_txt.readline().split(" ")[-1])
    Xmin = float(o_urbArea_t1_txt.readline().split(" ")[-1])
    Ymin = float(o_urbArea_t1_txt.readline().split(" ")[-1])
    cellsize = float(o_urbArea_t1_txt.readline().split(" ")[-1])
   
    o_urbArea_t1_txt.readline()

    for line in range(6):
        line = o_urbArea_t2_txt.readline()
        o_inFill_txt.write(line)
        o_otherNewDev_txt.write(line)
        
    Y = Ymin + ((nrows -1) * cellsize) + (cellsize / 2)

    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2
        
        urbArea_t1_line = o_urbArea_t1_txt.readline().split(" ")[:-1]
        urbArea_t2_line = o_urbArea_t2_txt.readline().split(" ")[:-1]
        infill_line = ""
        otherDev_line = ""


        for colNo in xrange(ncols):
            X+= cellsize

            urbArea_pix_t1 = urbArea_t1_line[colNo]
            urbArea_pix_t2 = urbArea_t2_line[colNo]

            if urbArea_pix_t2 in ['1','2','3']:   # if urban in t2
                if urbArea_pix_t1 in ['4','5']:   # if urbanized OS in t1
                    infill_line += "1 "
                    otherDev_line += "0 "
                    
                elif urbArea_pix_t1 in ['6','7']:
                    infill_line += "2 "
                    otherDev_line += "1 "

                else:
                    infill_line += "0 "
                    otherDev_line += "0 "

            else:
                infill_line += '0 '
                otherDev_line += '0 '
                    
        infill_line += "\n"
        otherDev_line += "\n"
        o_inFill_txt.write(infill_line)
        o_otherNewDev_txt.write(otherDev_line)

    o_urbArea_t1_txt.close()
    o_urbArea_t2_txt.close()
 
    o_inFill_txt.close()
    o_otherNewDev_txt.close()
        
    #-------------------------------------------------


    otherDev_img = "%s\\otherNewDev.img" % tempWksp
    gp.ASCIIToRaster_conversion (otherNewDev_txt, otherDev_img)



    otherDev_grp = "%s\\otherDev_grp.img" % tempWksp
    gp.RegionGroup_sa (otherDev_img, otherDev_grp, "EIGHT", "WITHIN")

    otherDev_grp_txt = "%s\\otherDev_grp_txt.txt" % tempWksp
    gp.RasterToASCII_conversion (otherDev_grp, otherDev_grp_txt)


    

    o_otherNewDev_txt = file(otherNewDev_txt, 'r')
    o_otherDev_grp_txt = file(otherDev_grp_txt, 'r')
    o_urbFtpt_t1 = file(urbFtpt_t1, 'r')
    
    

    ncols = int(o_otherNewDev_txt.readline().split(" ")[-1])
    nrows = int(o_otherNewDev_txt.readline().split(" ")[-1])
    Xmin = float(o_otherNewDev_txt.readline().split(" ")[-1])
    Ymin = float(o_otherNewDev_txt.readline().split(" ")[-1])
    cellsize = float(o_otherNewDev_txt.readline().split(" ")[-1])
    o_otherNewDev_txt.readline()

      
    for x in range(6):
        o_otherDev_grp_txt.readline()

    #------------------------------------
    # Create list of relative coordinates of pixels in neighborhood (center of neighborhood is 0,0)...

    winPosLst = [[-1,0],[1,0],[-1,1],[0,1],[1,1],[-1,-1],[0,-1],[1,-1]]
    #------------------------------------
       
         
    # get all lines from land cover txt file...
    lines = o_urbFtpt_t1.readlines()

    # omit header lines from urban landscape data list...
    lines = lines[6:]

    # get number of rows and columns for raster...
    ncols = len(lines[0].split(" ")[:-1])
    nrows = len(lines)

    # set initial min and max row number for neighborhood...
    minRow = -1
    maxRow = 1

    wRowDct = {}    # dictionary for neighborhood pixel values

    # populate dictionary with neighborhood pixel values...
    for wRowNo in xrange(0, maxRow+1):
        row = lines[wRowNo].split(" ")[:-1]     # extract row pixel values
        wRowDct[wRowNo] = row                   # add row pixel values to dictionary
     
    #------------------------------------
    # DETERMINE EDGE OPEN AREA (100 meter buffer around built-up)...

    rowCnt = 0
##        edgeCnt = totalPix = 0

    ext_dct = {}

    # for each row in image...
    for rowNo in xrange(nrows):

        o_newDev_line = o_otherNewDev_txt.readline().split(" ")[:-1]
        grp_line = o_otherDev_grp_txt.readline().split(" ")[:-1]
        
        # for each column in row...                                             
        for colNo in xrange(ncols):

            o_newDev_pix = o_newDev_line[colNo]

            ftpt_pix = wRowDct[rowNo][colNo]   # value for center pixel

            grp = grp_line[colNo]

            if o_newDev_pix == '1':

                if ftpt_pix in ['4','5']:
                    ext_dct[grp] = "1"
                else:
                         
                    # for each pixel in neighborhood...
                    for x,y in winPosLst:   # get relative coordinates

                        # try to get pixel value...
                        try:
                            # get pixel value from neighborhood data dictionary...
                            pix = wRowDct[rowNo+x][colNo+y]

                            #--------------------
                            # if pixel value is urban or suburban, center pixel is urbanized open
                            if pix in ["1","2","3"]:    # urban pixel
                                ext_dct[grp] = "1"
                                break
                            else:
                                continue
                        except: continue
                
        #--------------------------------   
        # Update neighborhood data dictionary - move down one row...

        # if min row is within image extent...
        if minRow > 0:
            del wRowDct[minRow] # delete top row

        # update min / max row...
        minRow += 1
        maxRow += 1

        # if max row is within extent...
        if maxRow < nrows:
            # add new max row to dictionary...
            wRowDct[maxRow] = lines[maxRow].split(" ")[:-1]

    # close text files... 
    o_otherNewDev_txt.close()

    o_urbFtpt_t1.close()

    del lines
    

    o_infill_txt = file(infill_txt, 'r')
    o_newDev_txt = file(newDev_txt, 'w')




                    

    #------------------------



    o_otherDev_grp_txt.seek(0)

    for line in range(6):
        o_newDev_txt.write(o_otherDev_grp_txt.readline())
        o_infill_txt.readline()
    
    for rowNo in xrange(nrows):

        X = Xmin - cellsize / 2
        
        grp_line = o_otherDev_grp_txt.readline().split(" ")[:-1]
        infill_line = o_infill_txt.readline().split(" ")[:-1]
        newDev_line = ""
      

        for colNo in xrange(ncols):
            X+= cellsize
         
            grp = grp_line[colNo]
            infill_pix = infill_line[colNo]

            if infill_pix == '1':
                newDev_line += "1 "     #infill
            elif infill_pix == '2':
                ext = ext_dct.get(grp)
                if ext == None:
                    newDev_line += "3 " #leapfrog
                else:
                    newDev_line += "2 " # extension
            else:
                newDev_line += "0 "

        newDev_line += "\n"
        o_newDev_txt.write(newDev_line)


    o_newDev_txt.close()
    o_otherDev_grp_txt.close()
    o_infill_txt.close()


#-------------------------------------------------
# GET EDGE PIXELS...

##        # Get edge pixel coordinates...
##        ncols = len(urbAreaGrid[0])
##        nrows = len(urbAreaGrid)
##        edgePix = []
##
##        for rowNo in xrange(nrows):
##
##            for colNo in xrange(ncols):
##                centerPix = urbAreaGrid[rowNo][colNo]
##                if centerPix == 0: continue
##                for x,y in [[-1,1],[0,1],[1,1],[-1,0],[1,0],[-1,-1],[0,-1],[1,-1]]:
##                    try:
##                        if urbAreaGrid[rowNo+y][colNo+x] == 0:
##                            edgePix.append(centerPix)
##                            break
##                    except: continue
##
#-------------------------------------------------



























## OLD ALGORITHM....

# CLASSIFY LAND COVER INTO THE FOLLOWING CATEGORIES:
#   (1) URBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness > 50%
#   (2) SUBURBAN ZONE BUILT-UP AREA: built-up pixels with imperviousness < 50% and > 10%
#   (3) RURAL ZONE BUILT-UP AREA: built-up pixels with imperviousness < 10%
#   (4) URBAN ZONE OPEN AREA: non-built-up pixels with imperviousness > 50%
#   (5) SUBURBAN ZONE OPEN AREA: non-built-up pixels with imperviousness < 50% and > 10%
#   (6) RURAL ZONE OPEN AREA: non-built-up pixels with imperviousness < 10%
#   (7) WATER: water
#   (8) EXCESSIVE SLOPE: slope > 15%

# Imperviousness defined as the % of imperviousness within a radius of 564 meters (circlular
# neighborhood with an area of 1 sq km)

### Requires text files for slope, land cover, and urban landscape. Requires neighborhood radius
##def urbLandscpe(slpTxtFile,lcTxtFile,urbLandscpe_txt,window_r):
##
##    #------------------------------------
##    # Create list of relative coordinates of pixels in neighborhood (center of neighborhood is 0,0)...
##
##    winPosLst = windowCoord(window_r)
##
##    #------------------------------------
##    # Prepare for % imperviousness calculations...
##
##    # open text files...  
##    o_lc = file(lcTxtFile)      # input: land cover
##    o_slp = file(slpTxtFile)    # input: slope
##    o_urbLandscape = file(urbLandscpe_txt, "w") # output: urban landscape
##            
##    # get cursor to correct line number...
##    for lineNo in range(6):
##        o_slp.readline()
##
##    # get all lines from land cover txt file...
##    lines = o_lc.readlines()
##
##    # write header lines to urban landscape txt file...
##    o_urbLandscape.writelines(lines[:6])
##
####    # get raster NoData value...
####    noData = lines[5].split(" ")[-1].rstrip("\n")
##
##    # omit header lines from land cover data list...
##    lines = lines[6:]
##
##    # get number of rows and columns for land cover raster (slope raster should have same extent)...
##    ncols = len(lines[0].split(" ")[:-1])
##    nrows = len(lines)
##
##    # set initial min and max row number for neighborhood...
##    minRow = -window_r
##    maxRow = window_r
##
##    wRowDct = {}    # dictionary for neighborhood pixel values
##
##    # populate dictionary with neighborhood pixel values...
##    for wRowNo in xrange(0, maxRow+1):
##        row = lines[wRowNo].split(" ")[:-1]     # extract row pixel values
##        wRowDct[wRowNo] = row       # add row pixel values to dictionary
##     
##    #------------------------------------
##    # CALCULATE % IMPERVIOUSNESS FOR EACH LAND COVER PIXEL (EXCEPT WATER AND EXCESS SLOPE)
##
##    rowCnt = 0
##
##    # for each row in image...
##    for rowNo in xrange(nrows):
##
##
##        urbLC_line = ""     # txt file line for urban landscape image
##        slp_line = o_slp.readline().split(" ")[:-1]     # corresponding line from slope txt file
##
##        # for each column in row...                                             
##        for colNo in xrange(ncols):
##
##            
##            centerPix = wRowDct[rowNo][colNo]   # value for center pixel
##
##            #----------------------------
##            # if center pixel is NoData ("0"), water ("2"), or excessive slope
##            # skip calculation. Update urban landscape txt file line...
##            
##            if centerPix not in ['1','2','3']:    # NoData 
##                urbLC_line += "0 "
##                continue
##            if centerPix == "2":    # water
##                urbLC_line += "7 "
##                continue
##
##            # REMOVE SLOPE FROM FINAL OUPTPUT...
##          
##            # excessive slope...
##            if float(slp_line[colNo]) > 15 and centerPix != 3: # slope > 15% is excessive
##                urbLC_line += "8 "
##                continue
##
##            
##
##            #----------------------------
##            # if center pixel is urban or other non-urban, calculate imperviousness...
##
##            urbCnt = pixCnt = 0     # count of neighborhood urban pixels and neighborhood total
##                                    # pixels (excluding NoData, water, excessive slope)
##
##            # for each pixel in neighborhood...
##            for x,y in winPosLst:   # get relative coordinates
##
##                # try to get pixel value...
##                try:
##                    # get pixel value from neighborhood data dictionary...
##                    pix = wRowDct[rowNo+x][colNo+y]
##
##                    #--------------------
##                    # if pixel value is urban, cnt as urban
##                    if pix == "3":      # urban pixel
##                        urbCnt += 1
##                    # if other non-urban, count pixel
##                    elif pix != "0":    # other non-urban pixel
##                        pixCnt += 1
##
##                # errors result from pixel being outside raster extent, skip these pixels
##                except:
##                    continue
##        
##            # calculate % imperviousness for pixel...
##            perUrb = float(urbCnt) / (urbCnt + pixCnt)               
##
##            #----------------------------
##            # Classify % imperviousness as urban (>0.5), suburban (<0.5 and > 0.1), or rural (<0.1)...
##
####            # % imperviousness > 0.5...
####            if perUrb > .66: 
####                if centerPix == "3":    # developed pixel
####                    urbLC_line += "1 "  # classify as URBAN
####                elif centerPix == "1":  # undeveloped pixel
####                    urbLC_line += "4 "  # classify as URBAN OS
####                    
####            # % imperviousness > 0.1 and < 0.5...        
####            elif .1 < perUrb <= .66:
####                if centerPix == "3":    # developed pixel
####                    urbLC_line += "2 "  # classify as SUBURBAN
####                elif centerPix == "1":  # undeveloped pixel
####                    urbLC_line += "5 "  # classify as SUBURBAN OS
####                    
####            # % imperviousness < 0.1...    
####            elif perUrb <= .1:
####                if centerPix == "3":    # developed pixel
####                    urbLC_line += "3 "  # classify as RURAL
####                elif centerPix == "1":  # undeveloped pixel
####                    urbLC_line += "6 "  # classify as RURAL OS
##
##            # if center pixel is built-up...
##            if centerPix == "3":
##                if perUrb > .5:
##                    urbLC_line += "1 "  #  urban
##                elif .1 < perUrb <= .5:
##                    urbLC_line += "2 "  # suburban
##                else:
##                    urbLC_line += "3 "  # rural
##
##            elif centerPix == "1":
##                if perUrb > .5:
##                    urbLC_line += "4 "  # urban open
##                elif .1 < perUrb <= .5:
##                    urbLC_line += "5 "  # suburban open
##                else:
##                    urbLC_line += "6 "  # rural open
##
##           
##
##        urbLC_line += "\n"
##
##        # update urban landscape text file...
##        o_urbLandscape.write(urbLC_line)
##
##        
##
##        #--------------------------------   
##        # Update neighborhood data dictionary - move down one row...
##
##        # if min row is within image extent...
##        if minRow > 0:
##            del wRowDct[minRow] # delete top row
##
##        # update min / max row...
##        minRow += 1
##        maxRow += 1
##
##        # if max row is within extent...
##        if maxRow < nrows:
##            # add new max row to dictionary...
##            wRowDct[maxRow] = lines[maxRow].split(" ")[:-1]
##
##    # close text files... 
##    o_lc.close()
##    o_slp.close()
##    o_urbLandscape.close()

