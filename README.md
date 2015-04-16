
# Redes2 - Practica 2

### Comandos utiles de `git`
* `git add .` - Añade ficheros al indice (pero no los que han sido eliminados, los nuevos mitah)
* `git add -A` - Añade TODOS los ficheros al indice (borrados, nuevos, modificados...)
* `git commit` - Consolida los cambios locales
* `git status` - Muestra el estado del indice
* `git push` - Sube los cambios al repositorio
* `git pull` - Se trae los cambios del repositorio
* `git mergetool` - Abre la herramienta definida en el fichero de conf para hacer merges
* `git checkout 8f2e -- fichero.txt` - Carga los ficheros tal y como estaban en el commit referido (o en el ultimo).
* `git checkout rama` - Cambia de rama.
* `git cherry-pick 8f2e` - Aplica el commit del argumento a la rama actual.
* `git merge rama` - Combina la rama del argumento con la actual y avanza la rama actual para que apunte al resultado del merge.
