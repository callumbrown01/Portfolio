'''

beaconSim.py - beacon simulation, knights move to the beacon when lit
               and run away when it goes out

'''

import matplotlib.pyplot as plt
import numpy as np
import random
from grailCast import *

MAXROWS = 40
MAXCOLS = 60

def flipCoords(row, col, limits):
    '''
    flipCoords - converts rows and columns to x,y coords for scatter plot

    row: row in grid
    col: column in grid
    limits: max number of rows and columns
    '''
    xpos = col
    ypos = limits[0] - row - 1
    return (xpos, ypos)
    
def plot_feature_scatter(itemlist, colour, limits):
    xlist = []
    ylist = []
    slist = []
    for r,c in itemlist:
        ylist.append(limits[0] - r - 1)  
        xlist.append(c)
        slist.append(100)
    plt.scatter(xlist,ylist,color=colour, marker='s', s=slist)
    
def plot_knight_scatter(knights, limits):
    xlist = []
    ylist = []
    slist = []
    clist = []
    for k in range(len(knights)):
        ylist.append(limits[0] - knights[k].getRow() - 1)  
        xlist.append(knights[k].getCol()) #flip rows/columns to y/x
        slist.append(40)
        clist.append(k)
    plt.scatter(xlist,ylist,s=slist,c=clist)
  

def main():

    knightNames = ["Sir Galahad", "Sir Robin"]
    beacons = [(20,30)] 
    beaconLit = False
    limits = [MAXROWS, MAXCOLS]
    # Starting population
    numKnights = 2
    knightList = []


    for i in range(numKnights):  # add knight objects to grid
        knightList.append(Knight(limits, knightNames[i])) 

    # Simulation
    
    for t in range(10):
        print("### Timestep ", t, "###")
        for i in range(numKnights):
            knightList[i].lure(beacons, limits)
        
        plot_knight_scatter(knightList, limits)
        plot_feature_scatter(beacons, "yellow", limits)
        plt.title("Beacon Simulation")
        plt.xlabel("Columns")
        plt.ylabel("Rows")
        plt.xlim(-1,MAXCOLS)
        plt.ylim(-1,MAXROWS)
        plt.pause(1)
        plt.clf()
    
if __name__ == "__main__":
    main()
