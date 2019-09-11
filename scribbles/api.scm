;(define* (frob foo #:optional (bar 42) (hello 100)  #:key (baz 73))
;  (list foo bar hello baz))

(use-modules (srfi srfi-9))
(define-record-type <gameobject>
  (make-gameobject name type position scale)
  gameobject?
  (name       get-name)
  (type       get-obj-type)
  (position   get-position)
  (scale      get-scale)
)

(define pawn (make-gameobject "testcam1" 'camera '(0 0 0) '(1 1 1)))


(define* (mkObj objName #:key (position '(0 0 0)) (scale '(0 0 0)) (rotation '(0 0 0)))
	(display "object name: ")
	(display objName)
	(display "\nposition: ")
	(display position)
	(display "\nscale: ")
	(display scale)
	(display "\nrotation: ")
	(display rotation)
	(display "\n")
)

(define (rmObj objName)
	(display (string-append "removing: " objName "\n"))
)

(define (movCam xoffset yoffset zoffset)
	(display (string-append "xoffset: " (number->string xoffset) "\n"))
	(display (string-append "yoffset: " (number->string yoffset) "\n"))
	(display (string-append "zoffset: " (number->string zoffset) "\n"))
)

(define (rotCam xoffset yoffset)
	(display (string-append "xoffset: " (number->string xoffset) "\n"))
	(display (string-append "yoffset: " (number->string yoffset) "\n"))
)

(define (lsObjByType objType)
	(display (list pawn))
	(display "\n")
)