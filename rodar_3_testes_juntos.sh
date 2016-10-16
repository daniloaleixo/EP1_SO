#!/bin/bash

python run_tests.py poucosProcessos/ arqs_saida/ arqs_deadline/ -t 1
python run_tests.py mediosProcessos/ arqs_saida/ arqs_deadline/ -t 1
python run_tests.py muitosProcessos/ arqs_saida/ arqs_deadline/ -t 1
