## Legacy Disk-IO B-Tree

Well, it's really sentimental to see this branch of my B-tree. 

It actually makes no sense to me to write something strange like *in-memory* B-tree (unless for
some pedagogical purpose, I would say). It's just as weird as something like
you're copying vectors around in Quicksort. Why not Red-Black trees instead?

Implementing a real disk-IO B-tree has been my goal at the beginning. And it was
*almost* finished but still problematic. But after testing, some difference between C++ `class` and C `struct`
made this implementation not usable in our RQRS. Unfortunately, I've already consumed
too much time in this course, so I gave up fixing this issue. :)