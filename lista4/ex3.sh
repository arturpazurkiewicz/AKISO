#!/bin/bash
echo $(curl -s http://api.icndb.com/jokes/random | jq -r  -a '.value.joke' )
curl -s $(curl -s --location --request GET "https://api.thecatapi.com/v1/images/search?format=json" \
  --header "Content-Type: application/json" \
  --header "x-api-key:c4337f5f-3d26-452c-b571-ef76d50fd97e" | jq -r -a '.[0].url' )> cat.jpg
catimg cat.jpg
