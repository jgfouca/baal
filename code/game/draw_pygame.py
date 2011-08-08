#! /usr/bin/env python

"""
API for drawing things in pygame.
"""

import unittest, pygame, os

from baal_common import prequire
from draw_mode import DrawMode, curr_draw_mode, _set_draw_mode
from baal_time import Time, Season
from geology import Geology, is_geological, \
    Divergent, Subducting, Orogenic, Transform, Inactive
from world_tile import WorldTile, \
    OceanTile, MountainTile, DesertTile, TundraTile, HillsTile, PlainsTile, \
    LushTile, FoodTile
from player import Player
from player_ai import PlayerAI
from weather import Atmosphere, Anomaly, is_atmospheric
from world import World

###############################################################################
class DrawPygame(object):
###############################################################################
    """
    Draw objects using pygame graphics.
    """

    SCREEN_WIDTH  = 800
    SCREEN_HEIGHT = 800

    #
    # ==== Public API ====
    #

    ###########################################################################
    def __init__(self):
    ###########################################################################
        self.__x_pos = 0
        self.__y_pos = 0

        # Initialize pygame
        pygame.init()
        self.__screen = pygame.display.set_mode((self.SCREEN_WIDTH,
                                                 self.SCREEN_HEIGHT))

        # Compute path to data area
        self.__path_to_data = os.path.abspath(
            os.path.join(os.path.dirname(__file__),
                         os.path.pardir, os.path.pardir, "data")
            )

        prequire(os.path.isdir(self.__path_to_data),
                 self.__path_to_data, " does not exist.")

    ###########################################################################
    def begin_draw(self):
    ###########################################################################
        self.__screen.fill( (0, 0, 0) ) # black

        self.__x_pos = 0
        self.__y_pos = 0

    ###########################################################################
    def end_draw(self):
    ###########################################################################
        # Pygame uses double-buffering; make the buffer we just drew into the
        # active buffer.
        pygame.display.flip()

    ###########################################################################
    def draw(self, item):
    ###########################################################################
        if (isinstance(item, Time)):
            self.__draw_time(item)
        elif (isinstance(item, Geology)):
            self.__draw_geology(item)
        elif (isinstance(item, Player)):
            self.__draw_player(item)
        elif (isinstance(item, PlayerAI)):
            self.__draw_player_ai(item)
        elif (isinstance(item, Atmosphere)):
            self.__draw_atmosphere(item)
        elif (isinstance(item, Anomaly)):
            self.__draw_anomaly(item)
        elif (isinstance(item, World)):
            self.__draw_world(item)
        elif (isinstance(item, WorldTile)):
            self.__draw_world_tile(item)
        else:
            prequire(False, "Class not drawable: ", item.__class__)

    #
    # ==== Internal Methods ====
    #

    ###########################################################################
    def __draw_time(self, item):
    ###########################################################################
        # Compute color
        if (item.season() == Season.WINTER):
            color = (0, 0, 255) # blue
        elif (item.season() == Season.SPRING):
            color = (0, 255, 0) # green
        elif (item.season() == Season.SUMMER):
            color = (255, 0, 0) # red
        elif (item.season() == Season.FALL):
            color = (255, 255, 0) # yellow
        else:
            prequire(False, "Unhandled season ", item.season())

        my_font = pygame.font.SysFont("None", # font name
                                      16)     # fontsize
        self.__screen.blit(my_font.render("%s, Year: %s" % (item.season(), item.year()),
                                          0, # antialias
                                          color),
                           (self.__x_pos + 20, self.__y_pos))

        self.__y_pos += 20
        self.__x_pos = 0

    # _GEOLOGY_MAP = {
    #     Divergent  : (BLUE,   "<-->"),
    #     Subducting : (RED,    "-v<-"),
    #     Orogenic   : (GREEN,  "-><-"),
    #     Transform  : (YELLOW, "vv^^"),
    #     Inactive   : (WHITE,  "")
    # }

    ###########################################################################
    def __draw_geology(self, item):
    ###########################################################################
        # draw_mode = curr_draw_mode()
        # prequire(is_geological(draw_mode), "Bad draw mode ", draw_mode)
        # expected_length = self.TILE_TEXT_WIDTH

        # # Figure out what string to print and what color it should be
        # if (draw_mode == DrawMode.GEOLOGY):
        #     color, symbol = self._GEOLOGY_MAP[item.__class__]

        #     to_draw = symbol.center(expected_length)
        # elif (draw_mode in [DrawMode.TENSION, DrawMode.MAGMA]):
        #     value = item.tension() if draw_mode == DrawMode.TENSION \
        #        else item.magma()
        #     to_draw = ("%.3f" % value).center(expected_length)

        #     if (value < .333):
        #         color = GREEN
        #     elif (value < .666):
        #         color = YELLOW
        #     else:
        #         color = RED
        # else:
        #     prequire(False, "Should not draw geology in mode: ", draw_mode)

        # prequire(len(to_draw) == self.TILE_TEXT_WIDTH,
        #          "Symbol '", to_draw, "' is wrong length")

        # cprint(color, to_draw)
        pass

    ###########################################################################
    def __draw_player(self, item):
    ###########################################################################
        player_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                      "player",
                                                      "baal.jpg"))
        self.__screen.blit(player_image,
            (self.__x_pos, self.__y_pos, 100, 100)) # allow 100x100 pixels

        self.__x_pos += 100

        my_font = pygame.font.SysFont("None", # font name
                                      16)     # fontsize

        self.__screen.blit(my_font.render("PLAYER STATS:",
                                   0, # antialias
                                   (255, 255, 255)), # white
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  name: %s" % item.name(),
                                   0, # antialias
                                   (255, 255, 255)), # white
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  level: %d" % item.level(),
                                   0, # antialias
                                   (0, 255, 0)), # green
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  mana: %d / %d" % (item.mana(), item.max_mana()),
                                   0, # antialias
                                   (0, 0, 255)), # blue
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  exp: %d / %d" % (item.exp(), item.next_level_cost()),
                                   0, # antialias
                                   (255, 255, 0)), # yellow
                    (self.__x_pos, self.__y_pos))

        self.__x_pos += 200
        self.__y_pos -= 80

    ###########################################################################
    def __draw_player_ai(self, item):
    ###########################################################################
        player_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                      "player",
                                                      "peasant.png"))
        self.__screen.blit(player_image,
            (self.__x_pos, self.__y_pos, 100, 100)) # allow 100x100 pixels

        self.__x_pos += 100

        my_font = pygame.font.SysFont("None", # font name
                                      16)     # fontsize

        self.__screen.blit(my_font.render("AI PLAYER STATS:",
                                   0, # antialias
                                   (255, 255, 255)), # white
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  tech level: %d" % item.tech_level(),
                                   0, # antialias
                                   (0, 255, 0)), # green
                    (self.__x_pos, self.__y_pos))
        self.__y_pos += 20

        self.__screen.blit(my_font.render("  population: %d" % item.population(),
                                   0, # antialias
                                   (0, 0, 255)), # blue
                    (self.__x_pos, self.__y_pos))

        self.__x_pos = 0
        self.__y_pos += 60

    # Describes how to draw the various fields. The first value in the pair
    # is the upper-bound for the corresponding color.
    # _MAX = 999999
    # _ATMOS_FIELD_COLOR_MAP = {
    #     DrawMode.WIND        : ((10, GREEN),  (20, YELLOW),   (_MAX, RED)),
    #     DrawMode.DEWPOINT    : ((32, RED),    (55, YELLOW),   (_MAX, GREEN)),
    #     DrawMode.TEMPERATURE : ((32, BLUE),   (80, YELLOW),   (_MAX, RED)),
    #     DrawMode.PRESSURE    : ((975, GREEN), (1025, YELLOW), (_MAX, RED)),
    #     DrawMode.RAINFALL    : ((2,  RED),    (10, YELLOW),   (_MAX, RED))
    # }

    ###########################################################################
    def __compute_atmos_color(self, draw_mode, field_value):
    ###########################################################################
        # for upper_bound, color in self._ATMOS_FIELD_COLOR_MAP[draw_mode]:
        #     if (field_value < upper_bound):
        #         return color

        # prequire(False,
        #          "Failed to find color for ", draw_mode, ", val ", field_value)
        pass

    ###########################################################################
    def __get_field_for_draw_mode(self, item, draw_mode):
    ###########################################################################
        if (draw_mode == DrawMode.WIND):
            return item.wind()
        elif (draw_mode == DrawMode.DEWPOINT):
            return item.dewpoint()
        elif (draw_mode == DrawMode.TEMPERATURE):
            return item.temperature()
        elif (draw_mode == DrawMode.PRESSURE):
            return item.pressure()
        elif (draw_mode == DrawMode.RAINFALL):
            return item.rainfall()
        else:
            prequire(False, "Bad draw mode: ", draw_mode)

    ###########################################################################
    def __draw_atmosphere(self, item):
    ###########################################################################
        # draw_mode = curr_draw_mode()

        # field = self.__get_field_for_draw_mode(item, draw_mode)
        # color = self.__compute_atmos_color(draw_mode, field)
        # str_  = str(field).center(self.TILE_TEXT_WIDTH)

        # cprint(color, str_)
        pass

    ###########################################################################
    def __draw_anomaly(self, item):
    ###########################################################################
        # print("Level:", item.intensity(), item.category(),
        #       "anomaly at location", item.location(),
        #       end="")
        pass

    ###########################################################################
    def __draw_world(self, item):
    ###########################################################################
        real_draw_mode = curr_draw_mode()

        self.draw(item.time())

        # Draw tiles
        for row in item.iter_rows():
            for tile in row:
                self.__draw_world_tile(tile)
            self.__x_pos = 0
            self.__y_pos += 100


        # Draw recent anomalies
        # for anomaly in item.iter_anomalies():
        #     self.draw(anomaly)
        #     print()

    _TILE_MAP = {
        OceanTile    : "ocean1.jpg",
        MountainTile : "mountain3.jpg",
        DesertTile   : "desert1.jpg",
        TundraTile   : "tundra1.jpg",
        HillsTile    : "hills1.jpg",
        PlainsTile   : "plains3.jpg",
        LushTile     : "grass1.jpg"
    }

    ###########################################################################
    def __draw_land(self, item):
    ###########################################################################
        filename = self._TILE_MAP[item.__class__]
        tile_image = pygame.image.load(os.path.join(self.__path_to_data,
                                                    "tiles",
                                                    filename))
        self.__screen.blit(tile_image,
            (self.__x_pos, self.__y_pos, 100, 100)) # allow 100x100 pixels
        self.__x_pos += 100

    ###########################################################################
    def __draw_world_tile(self, item, mode_override=None):
    ###########################################################################
        # if (mode_override is None):
        #     draw_mode = curr_draw_mode()
        # else:
        #     draw_mode = mode_override

        # if (draw_mode == DrawMode.LAND):
        #     self.__draw_land(item)

        # elif (draw_mode == DrawMode.CIV):
        #     if (item.city() is not None):
        #         cprint(RED, " C: ",
        #                str(item.city().rank()).center(self.TILE_TEXT_WIDTH-4))
        #     elif (item.infra_level() is not None and item.infra_level() > 0):
        #         cprint(YELLOW, " I: ",
        #                str(item.infra_level()).center(self.TILE_TEXT_WIDTH-4))
        #     else:
        #         self.__draw_land(item)

        # elif (draw_mode == DrawMode.MOISTURE):
        #     moisture = item.soil_moisture()
        #     if (moisture is not None):
        #         if (moisture < 1.0):
        #             color = YELLOW
        #         elif (moisture < FoodTile.FLOODING_THRESHOLD):
        #             color = GREEN
        #         elif (moisture < FoodTile.TOTALLY_FLOODED):
        #             color = BLUE
        #         else:
        #             color = RED
        #         cprint(color, "%.3f" % moisture)
        #     else:
        #         self.__draw_land(item)

        # elif (draw_mode == DrawMode.YIELD):
        #     yield_ = item.yield_()
        #     if (yield_.food > 0):
        #         cprint(GREEN, "%.3f" % yield_.food)
        #     else:
        #         cprint(RED, "%.3f" % yield_.prod)

        # elif (is_geological(draw_mode)):
        #     self.draw(item.geology())

        # elif (is_atmospheric(draw_mode)):
        #     self.draw(item.atmosphere())

        # else:
        #     prequire(False, "Unhandled mode: ", draw_mode)
        self.__draw_land(item)

#
# Tests
#

###############################################################################
class TestDrawPygame(unittest.TestCase):
###############################################################################

    ###########################################################################
    def test_draw_text(self):
    ###########################################################################
        # TODO
        pass

if (__name__ == "__main__"):
    unittest.main()
