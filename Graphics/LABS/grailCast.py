'''
grailCast.py - characters based on The Holy Grail, by Monty Python

Cast:
    Knight - the knights of the round table, lured by a beacon

'''
import random

class Knight():
    '''
    Knight - has a row,col position and a name

    __init__ arguments:
        row : within MAXROWS
        col : within MAXCOLS
        name : a string
    '''
    def __init__(self, limits, name):
        self.row = random.randint(0, limits[0]-1)
        self.col = random.randint(0, limits[1]-1)
        self.name = name
        self.captured = False
        self.felloff = False
    
    # Accessor methods

    def getRow(self):      # you *could* access these directly
        return self.row    # but showing how we can protect the data

    def getCol(self):
        return self.col

    def getName(self):
        return self.name

    # Mutator methods

    def lure(self, beacons, limits):
        '''
        lure - moves the individual knight towards the beacon(s)

        beacons - a list of beacon tuples (round bracket lists)
        limits - the boundaries of the "world"
        '''
        chosenBeacon = beacons[0] # could have multiple beacons...
        self.row -= 1
        self.col -= 1

    def runaway(self, beacons, limits):
        '''
        runaway - moves the individual knight away from the beacon(s)

        beacons - a list of beacon tuples (round bracket lists)
        limits - the boundaries of the "world"
        '''
        chosenBeacon = beacons[0]
        self.row += 1
        self.col += 1

