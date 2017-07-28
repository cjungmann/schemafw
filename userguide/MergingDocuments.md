# Merging Documents

This advanced topic guide is waiting for more usage experience in order to
get a handle on recommended practices.

The basic idea of a merged document is that a page view will include one or more
lookup results that can are useful to forms generated from the page.  In order for
the form to access the page-resident lookup results, it must be in the same document.
In a merge situation, the framework acquires the form data as normal, but then
copies the contents into the page document, tagging the merged elements to identify
them for transformation and for being discarded when finished.

This new method is being developed in the TestCase project, which may or not be
available when reading this.