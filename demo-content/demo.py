#! /usr/bin/python3
#
#
import argparse
import base64
import collections
import getpass
import json
import os
import platform
import random
import requests
import signal
import shutil
import string
from subprocess import Popen, PIPE

injection_input = 'injection-input.json'
directory_prefix= '/tmp/' + getpass.getuser() + '/cpp-injection/'
conversations_folder = 'conversations'
print(platform.system())
if platform.system() == 'Windows':
  binary = 'src/RelWithDebInfo/cpp_injection_demo'
else:
  binary = 'src/cpp_injection_demo'

def fetch_token(url):
    response = requests.get(url)
    if response.status_code == 200:
        try:
            token = response.json()['access_token']
            print(f'successfully fetched token from {url}')
            return token
        except Exception as exp:
            print(f'Unable to parse token from the token_server {exp}')
            print(f'You can acquire a client access token from the dashboard and place into the {injection_input} file')

def scan_assets():
    '''
    This method scans all assets under conversations folder
    '''
    convs = []
    folders = os.scandir(conversations_folder)
    for f in folders:
        if not f.name.startswith('.'):
            convs.append(f.name)

    convs.sort()
    if len(convs) == 0:
        print('No conversation assets found, check "conversations" folder to see if it is in the same folder as the demo.py script')
        exit()

    return convs

def get_ext_id(x, y, z, r, name, b):
    '''
    The conversation definition file position and direction inputs are used to construct the injection bot's external ID,
    which can be parsed by the client side application like Unity, Unreal and Web demo to place the bots in the right location with the right direction
    '''
    ext = {
        'init-pos': {'x':x, 'y':y, 'z': z, 'r': r},
        't1': b['t1'] if 't1' in b else '',
        't2': b['t2'] if 't2' in b else '',
        'idx': ''.join(random.choices(string.ascii_lowercase + string.digits, k=4))
    }
    return base64.b64encode(json.dumps(ext).encode('utf-8')).decode('utf-8')

def stop_injection_process(folder):
    if os.path.exists(folder):
        injector = open(folder + "/pid", "r")
        pid = injector.read()
        os.kill(int(pid), signal.SIGTERM)

def parse_injection_input():
    '''
    This method scans the injection input file and validates the input
    '''
    with open(injection_input, 'r') as input_file:
        content = json.loads(input_file.read())
        try:
            token_server_url = content['access']['token_server_url']
            client_access_token = content['access']['client_access_token']
            alias = content['conf_alias']
            conversations = content['conversations']
            style = content['spatial']['style']
            scale = str(content['spatial']['scale']['x']) + ";" + str(content['spatial']['scale']['y']) + ";" + str(content['spatial']['scale']['z'])
            right = str(content['spatial']['right']['x']) + ";" + str(content['spatial']['right']['y']) + ";" + str(content['spatial']['right']['z'])
            up = str(content['spatial']['up']['x']) + ";" + str(content['spatial']['up']['y']) + ";" + str(content['spatial']['up']['z'])
            forward = str(content['spatial']['forward']['x']) + ";" + str(content['spatial']['forward']['y']) + ";" + str(content['spatial']['forward']['z'])
            if len(token_server_url) > 0:
                token = fetch_token(token_server_url)
                if token is not None:
                    client_access_token = token

            if len(client_access_token) == 0:
                print(f'You have not specified any valid input for acquiring client access token, please review the settings in {injection_input}\n')
                print('You have two options to acquire a valid token:')
                print('1 - get one from the dolby.io dashboard and place under the "client_access_token" field')
                print('2 - setup a token generation server and provide the URL under "the token_server_url" field')
                exit()

            if len(conversations) == 0:
                print(f'You must add a few conversations to the {injection_input}, they can be added as two digit index separated by comma. For example "00,01"')
                exit()

            if len(alias) == 0:
                print(f'You have not specified conference alias in {injection_input}, default value "demo" will be used')
                alias = 'demo'
            
            if style.lower() not in ['shared', 'individual', 'none']:
                print(f'You have not specified a valid input [{spatial_style}] for "spatial_style", default value "shared" will be used')
                spatial_style = 'shared'

            return client_access_token, alias, conversations, style, scale, right, up, forward 

        except Exception as exp:
            print(f'Failed parsing injection input file: {exp}')

def collect_commands(conversation, alias, client_access_token, style, scale, right, up, forward, args):
    folder = f'{conversations_folder}/{conversation}/'
    def_json = f'{folder}def.json'
    cmds = []
    if os.path.exists(def_json):
        with open(def_json, 'r') as r:
            j = json.loads(r.read())
            for b in j:
                x = b['x']
                z = b['z']
                y = b['y']
                r = b['r']
                name = b['name']
                directory = directory_prefix + conversation + "/" + name
                if not os.path.exists(directory):
                    os.makedirs(directory)
                ext_id = get_ext_id(x, y, z, r, name, b)
                spatial_style = f' -spatial {style}' if style != 'none' else ''
                if not args.stop:
                    f = b['media']
                    f = f'{folder}{f}'
                    media = 'A' if '.aac' in f or '.wav' in f or '.m4a' in f else 'AV'
                    cmd = f'./{binary} -c {alias} -k {client_access_token} -l 3 -ld {directory} -initial-spatial-position {x};{y};{z} -initial-yaw-rotation {r} -initial-scale {scale} -u {name} -e {ext_id} -p user -m {media} --enable-media-io -f {f} -loop{spatial_style} -initial-right {right} -initial-up {up} -initial-forward {forward}'.split(' ')
                    cmds.append(Popen(cmd))
                else:
                    stop_injection_process(directory)
    return cmds

def setup_conference(client_access_token, alias, conversations, style, scale, right, up, forward, args):
    '''
    This method scans the assets and use injection input json parameters to construct the injection command
    '''
    print(f'About to inject media into "{alias}"')
    assets = scan_assets()
    if len(conversations) > 0:
        commands = []
        conversations = conversations.split(',')
        for c in conversations:
            found = False
            for conversation in assets:
                if conversation.startswith(c):
                    found = True
                    commands += collect_commands(conversation, alias, client_access_token, style, scale, right, up, forward, args)
            
            if not found:
                print(f'Error - invalid conversation index specified {c}, request ignored')
        
        print(f'injecting {len(commands)} bots')
        for p in commands:
            p.wait()

def setup_cli():
    parser = argparse.ArgumentParser()
    parser.add_argument('-stop', default='', help='stop active injections for selected folders')
    parser.add_argument('-clear', default='', help='clear the log files and such for the previous injections')
    return parser.parse_args()

args = setup_cli()
if not args.clear:
    client_access_token, alias, conversations, style, scale, right, up, forward = parse_injection_input()
    setup_conference(client_access_token, alias, conversations, style, scale, right, up, forward, args)
else:
    if os.path.exists(directory_prefix):
        shutil.rmtree(directory_prefix)
