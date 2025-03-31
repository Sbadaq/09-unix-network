find . -name "*.c" -exec git add {} \; 
find . -name "*.h" -exec git add {} \; 
find . -name "*.MD" -exec git add {} \;
find . -name "*.md" -exec git add {} \; 
git commit -m "$1"
git push origin master
