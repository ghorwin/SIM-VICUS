# Contributing to SIM-VICUS

SIM-VICUS is a community project and it can only benefit from contributions of all kinds. So, thanks for contributing to the project! :tada::+1:

The following is a set of guidelines for contributing to SIM-VICUS, the NANDRAD solver and associated documentation, which are hosted at 
[SIM-VICUS Project Page](https://github.com/sim-vicus) on GitHub. These are mostly guidelines, not rules. Use your best judgment, and feel 
free to propose changes to this document in a pull request.

## Code of Conduct

This project and everyone participating in it is governed by the [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to [sim-vicus@listserv.dfn.de](mailto:sim-vicus@listserv.dfn.de).

## Sources for Information

The best place to for contributers start is the [documentation page](https://ghorwin.github.io/SIM-VICUS). Please also join the mailing list https://www.listserv.dfn.de/sympa/info/sim-vicus and discuss ideas and ask questions there.

### SIM-VICUS and NANDRAD

The repository basically includes several distinct software packages (and some utility scripts, and build system tools, and and and..). But the central pieces are:

* **NandradSolver**, a command line multi-zone building and thermo-hydraulic network model solver
* **SIM-VICUS**, the graphical user interface for NANDRAD (and potentially other solvers/tools)
* **NandradFMUGenerator**, a user interface (with automatition-via-command-line) for generating FMUs from NANDRAD projects

Also, there are utilities being generated, like `View3D`.

### Design Decisions

Especially the user interface needs a clean and consistent design. We define the design together in group discussions and try to come up with a good user interface experience. This may mean that sometimes initial implementations need to be revised or changed, but with help of the community, this need not be a major workload issue. Generally, before starting any larger work on the user interface with new design ideas in mind, an early discussion of the ideas is encouraged.

## How Can I Contribute?

### Reporting Bugs

Submit bugs on the issues page: https://github.com/ghorwin/SIM-VICUS/issues
Please search the pages first if something similar exists, already, and comment on existing issues rather opening new ones.

> **Note:** If you find a **Closed** issue that seems like it is the same thing that you're experiencing, open a new issue and include a link to the original issue in the body of your new one.

#### How Do I Submit A (Good) Bug Report?

Bugs are tracked as [GitHub issues](https://guides.github.com/features/issues/). Explain the problem and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem (suitable to be used directly in changelogs; maintainers should improve the title if needed).
* **Describe the exact steps which reproduce the problem** in as many details as possible. When listing steps, **don't just say what you did, but explain how you did it**. For example, if you used an interactive transformation operation, describe how you started the transformation with the mouse, what position the local coordinate system had and what happened, when you stopped dragging the mouse cursor.
* **If possible, provide specific examples to demonstrate the steps**. Include links to files or GitHub projects, or copy/pasteable snippets, which you use in those examples. If you're providing snippets in the issue, use [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
* **Provide complete NANDRAD projects including referenced climate data files** when reporting bugs/crashes in the NANDRAD solver core.
* **Explain which behavior you expected to see instead and why.**
* **Include screenshots and animated GIFs** which show you following the described steps and clearly demonstrate the problem. You can use [LICEcap](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [Silent cast](https://github.com/colinkeenan/silentcast) or _Shutter_ on Linux.
* **If the problem wasn't triggered by a specific action**, describe what you were doing before the problem happened and share more information using the guidelines below.

Provide more context by answering these questions:

* **Did the problem start happening recently** (e.g. after updating to a new version of SIM-VICUS/NandradSolver) or was this always a problem?
* If the problem started happening recently, **can you reproduce the problem in an older version as well?** What's the most recent version in which the problem doesn't happen?
* **Can you reliably reproduce the issue?** If not, provide details about how often the problem happens and under which conditions it normally happens.

Include details about your configuration and environment:

* **Which version of SIM-VICUS/NandradSolver are you using?** You can get the exact version by running `NandradSolver --version` in your terminal, or by starting SIM-VICUS and checking the program version in the application title (or welcome screen).
* **What's the name and version of the OS you're using**?
* **Are you running SIM-VICUS in a virtual machine?** If so, which VM software are you using and which operating systems and versions are used for the host and the guest? (OpenGL-errors may be a direct result of the VM configuration)
* **Does the problem disappear with a fresh installation (i.e. cleared user configuration/database file)

### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion, including completely new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion :pencil: and find related suggestions :mag_right:.

#### How Do I Submit A (Good) Enhancement Suggestion?

Enhancement suggestions are tracked as [GitHub issues](https://guides.github.com/features/issues/). 

* **Use a clear and descriptive title** for the issue to identify the suggestion.
* **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
* **Provide specific examples to demonstrate the steps**. Include copy/pasteable snippets which you use in those examples, as [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
* **Include screenshots and animated GIFs** which help you demonstrate the steps or point out the part of Atom which the suggestion is related to. You can use [this tool](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **Specify the name and version of the OS you're using.**

### Your First Contribution - License Regulations

SIM-VICUS and all code in the SIM-VICUS repo is placed under an open-source license. Hence, by providing your contributions they are automatically placed under the same license, such that SIM-VICUS and all code in the repository remains fully open-source. You as contributor must ensure that your contributions are free of license restrictions of third-party code included in your contribution.

### Commit guidelines

Changes to the code base must ensure that the entire code compiles. Maintainers should check that after pushing changes the code builds on all platforms and the regression tests run through.

Make small commits centered around a single feature. Feature additions involving data model changes, solver changes and UI changes should be split into different commits and pushed in the order of dependencies. 

When changing *and* extending a piece of code at the same time, consider creating two commits - one that identifies the fix/change, and one that identifies the extension.

#### Commit messages

* Use the past tense ("Added feature", "Changed ...")
* When you close an issue, use the phrase "closed #232" or "fixed #222" in your commit message. This will automatically close the issue in github.
* Try to avoid line breaks in the list of changes in your commit message. This makes it easier to find changes when looking in the compact commit list.

### Pull Requests

The process described here has several goals:

- Maintain SIM-QUALITY's and NANDRAD's quality
- Fix problems that are important to users
- Engage the community in working toward the best possible software
- Enable a sustainable system for the maintainers to review contributions

The basic rule of thumb is: make code reviews simple for maintainers. That includes:

- follow the [styleguides](#styleguides)
- provide good commit messages
- create a ticket with enough information and reference it in your commit messages
- personal communication (email) with the maintainers via mailing list should be done
- model extensions require matching documentation update in the same commit (both source code and data model documentation) - we do not want to add undocumented features into the solver model
- be prepared to review/rework your code and don't expect maintainers to patch everthing up (but don't be annoyed if maintainers change your code to apply coding style guides)

## Styleguides

See [coding guidelines](https://ghorwin.github.io/SIM-VICUS/Developer-Documentation/index.html#_coding_guidelines_and_rules) in the developer docs.

These rules are not set in stone, and above all stands the rule: __make the code easy to read, understand and maintain__!
