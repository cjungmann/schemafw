# Creating an SRM Emacs Mode

The main reason for creating an Emacs mode for editing SRM files is the
challenge of keeping related instructions indented to the same level.
There are secondary benefits as well, including being able to define keywords
that are then highlighted in the Emacs editor to help detect keyword spelling
errors.

I found this [article on GenericMode](https://www.emacswiki.org/emacs/GenericMode) in the [EmacsWiki](https://www.emacswiki.org/).  This page described how
to make the new mode.  The tab stops seem to be taken from the global variable
*tab-width* (`(setq-default tab-width 3)` in my *init.el* file).

This is my first attempt to create a sfw-mode.  I copied the following code
into *~/.emacs.d/init.el* near the bottom of the file.

~~~
(require 'generic-x)                          ;; must exist to derive from generic
(define-generic-mode
  ;; mode name parameter:
  'sfw-mode
  ;; comment start/end parameter:
  '("#")
  ;; keywords parameter:
  '("schema" "type" "procedure" "schema-proc" "$database" "$xml-stylesheet"
    "$default-mode" "button" "label" "task" "on_line_click" "form-action"
     "filename" "url" "result" "confirm")
  ;; other highlighting parameter:
  '(("form-\\(new\\|edit\\|view\\|submit\\)" . font-lock-constant-face)
    ("\\(delete\\|update\\|add\\|export\\)" . font-lock-constant-face)
   )
  ;; filename templates parameter (files that activate this mode):
  '("\\.srm$")
  ;; other functions-to-call parameter:
  nil
  ;; mode description string
  "A mode for Schema Framework SRM files"
)

;; Failed attempt to get Shift-tab to unindent:
;; (add-hook 'sfw-mode-hook
;;           (lambda () (local-set-key (kbd "<backtab>") 'backward-to-indentation)))
~~~

Sadly, as you can see from the commented out *local-set-key* function above, I have not been able to
find a concise function to unindent lines in a Pythonic manner.  It would be a nice feature here, but
at least being able to indent to tab-stops is an improvement to having to count spaces or move blocks
around to match indentation levels.

A final note on the commented *local-set-key* setting: it seems that the syntax of the *add-hook*
command works...when using `(kbd "S-<tab>")` etc, Emacs warning of there being no binding for
*<backtab>*.  This warning went away when I changed it to *<backtab>*, so the problem here seems to
be that *backward-to-indentation* doesn't do anything.  The above statement may still work if an
appropriate function can be found.

