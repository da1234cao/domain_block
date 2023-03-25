#!/usr/bin/env bash

systemctl daemon-reload  
systemctl start domain_block

echo "postinst end.\n"