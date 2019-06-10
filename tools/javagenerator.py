# Copyright 2019 Brian T. Park
#
# MIT License


class JavaGenerator:
    """Create a Java source file that contains the names of zones
    supported by zonedb and zonedbx. Will be used by the Java program
    to generate the validation data beyond the 2038 limit of pytz.
    """
