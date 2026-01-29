To generate the pdf run the following command in the joss_paper directory: 

`docker run --rm --volume "$PWD":/data --workdir /data --user $(id -u):$(id -g) 
--env JOURNAL=joss openjournals/inara`

