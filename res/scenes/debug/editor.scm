
(define (findIndex theList element currIndex) 
  (if (equal? (length theList) 0)
    #f
    (if (equal? (car theList) element)
      currIndex
      (findIndex (cdr theList) element (+ currIndex 1))
    )
  )
)

(define panelTypes (list "one" "two" "three"))
(define currPanelObj #f)

(define (showSidePanel panelType)
  (if (findIndex panelTypes panelType 0)
    (begin
      (if currPanelObj
        (rm-obj (gameobj-id (lsobj-name currPanelObj)))
      )
      (let ((objname (string-append "$panel" panelType)))
        (set! currPanelObj objname)
        (mk-obj-attr 
          objname
          (list
            (list "file" (string-append "./res/scenes/debug/" panelType ".rawscene"))
          )
        )
      )
    )
  )
)

(define (onMessage key value)
  (if (equal? key "editor-showpanel")
    (showSidePanel value)
  )
  (display "key is: ")
  (display key)
  (display " value: ")
  (display value)
  (display "\n")
)


