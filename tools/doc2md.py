#!/usr/bin/env python2.7
import json
from pprint import pprint
import os
import sys
import re

import dokumentor
import subprocess

def parseParam(arg, indent=0, isReturn=False):
    out = ""
    if isReturn:
        out += "Returns (%s): %s\n" % (parseParamsType(arg["typed"]), arg["description"])
    else:
        out += "%s* `%s` (%s): %s\n" % (' ' * indent, arg["name"], parseParamsType(arg["typed"]), arg["description"])

    if "params" in arg:
        # Callback function
        for subArg in arg["params"]:
            out += parseParam(subArg, indent + 4)
    elif type(arg["typed"][0]) is dict:
        # Object
        for subArg in arg["typed"][0]["details"]:
            out += parseParam(subArg, 0 if isReturn else indent + 4)
    elif type(arg["typed"][0]) is list:
        # Array of Object
        for subArg in arg["typed"][0][0]["details"]:
            out += parseParam(subArg, 0 if isReturn else indent + 4)

    return out

def parseParamsType(types):
    out = ""
    comma = ""
    for t in types:
        out += comma
        if type(t) is list:
            out += "Object[]"
        elif type(t) is dict:
            out += "Object"
        else:
            if t[0] == "[":
                out += t[1:-1].capitalize() + "[]"
            else:
                if t == "null":
                    out += t
                else:
                    out += t if t[0].isupper() else t.capitalize()
        comma = " | "

    return out

def parseMethod(method, isEvent=False):
    out = ""

    if isEvent:
        out += "\n## Event: %s\n" % (re.sub("[A-Za-z_0-9]+\.", "", method["name"]))
    else:
        fnArgs = ""

        if len(method["params"]) > 0:
            comma = ""
            for arg in method["params"]:
                name = comma + arg["name"]

                if arg["default"] != "None":
                    name += "=%s" % arg["default"]

                if arg["is_optional"]:
                    name = "[%s]" % name

                fnArgs += name
                comma = ", "

        out += "\n## %s%s%s(%s)\n" % ("`static `" if method["is_static"] else "", 
                                    "new " if method["is_constructor"] else "",
                                    method["name"],
                                    fnArgs)
        if method["is_slow"]:
            out += "<!-- YAML\n- Slow method\n-->\n"

    out += method["description"] + "\n"

    if len(method["params"]) > 0:
        out += "\nParams:\n"
        for arg in method["params"]:
            out += parseParam(arg)

    if method["returns"] and not method["is_constructor"]:
        if method["returns"]["nullable"]:
            method["returns"]["typed"].append("null")

        tmp = parseParam(method["returns"], isReturn=True)
        if tmp:
            out += "\n" + tmp

    out += parseSeeAlso(method["sees"])

    return out

def parseProperty(prop):
    out = ""
    out += "\n## %s%s%s%s (%s)\n" % ("`static` " if prop["is_static"] else "",
                              "`readonly` " if prop["is_readonly"] else "",
                              prop["name"],
                              "=" + prop["default"] if prop["default"] != "None" else "",
                              parseParamsType(prop["typed"]))

    out += prop["description"] + "\n"
    out += parseExample(prop["examples"])
    out += parseSeeAlso(prop["sees"])

    return out

def parseSeeAlso(seeAlso):
    return ""
"""
    out = ""
    if len(seeAlso) > 0:
        out += "\nSee also:\n"
        for see in seeAlso:
            out += "* `%s`\n" % (see["data"])

    return out
"""

def parseExample(examples):
    out = ""
    if len(examples) > 0:
        out += "\n"
        for ex in examples:
            out += "\n```%s\n%s\n```\n"  % (ex["language"], ex["data"])

    return out

def parse(klass, data):
    out = ""
    out += "# Class: %s" % (klass) + "\n"
    item = data["base"][klass]

    out += item["description"]

    out += parseExample(item["examples"])

    out += parseSeeAlso(item["sees"])

    if data["constructors"]:
        out += parseMethod(data["constructors"][klass])

    if data["methods"]:
        for name, method in data["methods"].iteritems():
            out += parseMethod(method)

    if data["static_methods"]:
        for name, method in data["static_methods"].iteritems():
            out += parseMethod(method)

    if data["properties"]:
        for name, prop in data["properties"].iteritems():
            out += parseProperty(prop)

    if data["events"]:
        for evName, ev in data["events"].iteritems():
            out += parseMethod(ev, isEvent=True)

    return out


print("Running dokumentor")
class captureDokumentor:
    def __init__(self):
        self.data = ""

    def write(self, msg):
        self.data += msg

    def flush(self=None):
        pass

sys.stdout = captureDokumentor()
dokumentor.process("../docs/")
docs = sys.modules['DOCC'].DOC
dokumentor.report("json", docs)
data = json.loads(sys.stdout.data)
sys.stdout = sys.__stdout__

hierarchy = {}
for section, items in data["_sections"].iteritems():
    if section not in data:
        data[section] = {"base": { section: {"description": "", "sees":[], "examples": {}}}, "constructors": {}, "methods": [], "properties": [], "events":[], "static_methods": []}
    hierarchy[section] = {"data": parse(section, data[section])}
    hierarchy[section]["children"] = {}
    for klass in items:
        hierarchy[section]["children"][klass] = parse(klass, data[klass])

path = "../docs/en/api/"
try:
    os.mkdir(path)
except:
    pass

for directory in hierarchy:
    if len(hierarchy[directory]["children"]) > 1:
        subPath = path + directory + "/"
        try:
            os.mkdir(subPath)
        except:
            pass
        print("Writing %s" % subPath + directory + ".md")
        with open(subPath + directory + ".md", "w") as f:
            f.write(hierarchy[directory]["data"])

        for child in hierarchy[directory]["children"]:
            print("     - Writing %s" % subPath + child + ".md")
            with open(subPath + child + ".md", "w") as f:
                f.write(hierarchy[directory]["children"][child])
    else:
        print("Writing %s" % path + directory + ".md")
        with open(path + directory + ".md", "w") as f:
            f.write(hierarchy[directory]["data"])
        
