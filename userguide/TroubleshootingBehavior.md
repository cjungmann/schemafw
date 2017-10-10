# Troubleshooting Behavior

If you can see an error message, refer also to [Troubleshooting SchemaFW Messages](Troubleshooting.md).

In developing this framework, I have occasionally encountered frustrating and
confusing behaviors that took a long time to solve.  This page will list some of these
problems, their causes and solutions.

## Unexplained Sudden Jump to a New Page

### Situation

I was developing the **iltable** control that hosts a table in a from.  The
*iltable* element included a "plus" button with which the user could add a new element
to the table.

Although code execution could be traced in the debugger right after the button
was pressed, when the last of the code returned, the page refreshed to a new empty
page.

### Problem

I eventually noticed that the new URL was a form submission page.  The form was
being unexpectedly submitted.

### The Solution

I eventually found an [explanation](https://stackoverflow.com/questions/11893573/what-triggers-an-html-form-to-submit)
that helped me solve the problem.  To paraphrase (in case the link expires), the
"plus" button did not define the type.  I surmise that since it was hosted by a form,
the button was treated as a submit button.  I make this conclusion, that being hosted
in a form is a contributing factor, because other un-typed buttons don't jump anywhere.

In any event, I ensured that the button included a type attribute set to "button",
and the undesirable jump stopped.
